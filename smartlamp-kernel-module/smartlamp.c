#include <linux/module.h>
#include <linux/usb.h>
#include <linux/slab.h>

MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Driver de acesso ao SmartLamp (ESP32 com Chip Serial CP2102");
MODULE_LICENSE("GPL");


#define MAX_RECV_LINE 100 // Tamanho máximo de uma linha de resposta do dispositvo USB

static char recv_line[MAX_RECV_LINE];              // Armazena dados vindos da USB até receber um caractere de nova linha '\n'
static struct usb_device *smartlamp_device;        // Referência para o dispositivo USB
static uint usb_in, usb_out;                       // Endereços das portas de entrada e saida da USB
static char *usb_in_buffer, *usb_out_buffer, *buffer;       // Buffers de entrada e saída da USB
static int usb_max_size;                           // Tamanho máximo de uma mensagem USB
static int usb_read_serial(char* match);
static char* usb_write_serial(char *cmd, int param);

#define VENDOR_ID   0x10c4 /* VendorID  do smartlamp */
#define PRODUCT_ID  0xea60 /* ProductID do smartlamp */
static const struct usb_device_id id_table[] = { { USB_DEVICE(VENDOR_ID, PRODUCT_ID) }, {} };

static int  usb_probe(struct usb_interface *ifce, const struct usb_device_id *id); // Executado quando o dispositivo é conectado na USB
static void usb_disconnect(struct usb_interface *ifce);                           // Executado quando o dispositivo USB é desconectado da USB
static int usb_send_cmd(char *cmd, int param);
  

// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é lido (e.g., cat /sys/kernel/smartlamp/led)
static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff);
// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é escrito (e.g., echo "100" | sudo tee -a /sys/kernel/smartlamp/led)
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count);   
// Variáveis para criar os arquivos no /sys/kernel/smartlamp/{led, ldr}
static struct kobj_attribute  led_attribute = __ATTR(led, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct kobj_attribute  ldr_attribute = __ATTR(ldr, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct kobj_attribute  temp_attribute = __ATTR(temp, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct kobj_attribute  hum_attribute = __ATTR(hum, S_IRUGO | S_IWUSR, attr_show, attr_store);
static struct attribute      *attrs[]       = { &led_attribute.attr, &ldr_attribute.attr, &temp_attribute.attr, &hum_attribute.attr,  NULL };
static struct attribute_group attr_group    = { .attrs = attrs };
static struct kobject        *sys_obj;                                             // Executado para ler a saida da porta serial

MODULE_DEVICE_TABLE(usb, id_table);

bool ignore = true;
int LDR_value = 0;

static struct usb_driver smartlamp_driver = {
    .name        = "smartlamp",     // Nome do driver
    .probe       = usb_probe,       // Executado quando o dispositivo é conectado na USB
    .disconnect  = usb_disconnect,  // Executado quando o dispositivo é desconectado na USB
    .id_table    = id_table,        // Tabela com o VendorID e ProductID do dispositivo
};

module_usb_driver(smartlamp_driver);

// Executado quando o dispositivo é conectado na USB
static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id) {
    struct usb_endpoint_descriptor *usb_endpoint_in, *usb_endpoint_out;

    printk(KERN_INFO "SmartLamp: Dispositivo conectado ...\n");

    // Cria arquivos do /sys/kernel/smartlamp/*
    sys_obj = kobject_create_and_add("smartlamp", kernel_kobj);
    ignore = sysfs_create_group(sys_obj, &attr_group); // AQUI

    // Detecta portas e aloca buffers de entrada e saída de dados na USB
    smartlamp_device = interface_to_usbdev(interface);
    ignore =  usb_find_common_endpoints(interface->cur_altsetting, &usb_endpoint_in, &usb_endpoint_out, NULL, NULL);  // AQUI
    usb_max_size = usb_endpoint_maxp(usb_endpoint_in);
    usb_in = usb_endpoint_in->bEndpointAddress;
    usb_out = usb_endpoint_out->bEndpointAddress;
    usb_in_buffer = kmalloc(usb_max_size, GFP_KERNEL);
    usb_out_buffer = kmalloc(usb_max_size, GFP_KERNEL);
    buffer = kmalloc(usb_max_size, GFP_KERNEL);

    LDR_value = usb_send_cmd("SET_LED", 80);
    printk(KERN_INFO "LED Value: %d", LDR_value);
    LDR_value = usb_send_cmd("GET_LDR", -1);
    printk(KERN_INFO "LDR Value: %d", LDR_value);

    return 0;
}

// Executado quando o dispositivo USB é desconectado da USB
static void usb_disconnect(struct usb_interface *interface) {
    printk(KERN_INFO "SmartLamp: Dispositivo desconectado.\n");
    if (sys_obj) kobject_put(sys_obj);      // Remove os arquivos em /sys/kernel/smartlamp
    kfree(usb_in_buffer);                   // Desaloca buffers
    kfree(usb_out_buffer);
}
static int get_int_from_buffer(const char* buffer, const char* target_string) {
    char *ptr = strstr(buffer, target_string);
    int i = 0;

    if (ptr != NULL) {
        ptr += strlen(target_string);
        int result = 0;
        while ('0' <= *ptr && *ptr <= '9') {
            result *= 10;
            result += *ptr - '0';
            ptr++;
        }
        return result;
    }

    return -9999;
}

static int usb_send_cmd(char *cmd, int param) {
    int recv_size = 0;                     // Quantidade de caracteres no recv_line
    int ret_read , actual_size, i;
    char* ret_write;
    int retries = 10;                       // Tenta algumas vezes receber uma resposta da USB. Depois desiste.
    char resp_expected[MAX_RECV_LINE];      // Resposta esperada do comando
    char *resp_pos;                         // Posição na linha lida que contém o número retornado pelo dispositivo
    long resp_number = -1;                  // Número retornado pelo dispositivo (e.g., valor do led, valor do ldr)

    printk(KERN_INFO "SmartLamp: Enviando comando: %s\n", cmd);

    ret_write = usb_write_serial(cmd, param);
    if (ret_write == NULL) {
        return -1;
    }

    ret_read = usb_read_serial(ret_write);
    if(ret_read != -1){
        return ret_read;
    }
    return -1; // Não recebi a resposta esperada do dispositivo
}

// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é lido (e.g., cat /sys/kernel/smartlamp/led)
static ssize_t attr_show(struct kobject *sys_obj, struct kobj_attribute *attr, char *buff) {
    // value representa o valor do led ou ldr
    int value = -1;

    // attr_name representa o nome do arquivo que está sendo lido (ldr ou led)
    const char *attr_name = attr->attr.name;

    // printk indicando qual arquivo está sendo lido
    printk(KERN_INFO "SmartLamp: Lendo %s ...\n", attr_name);

    if (strcmp(attr_name, "led") == 0) {
        value = usb_send_cmd("GET_LED", -1);
    }
    else if(strcmp(attr_name, "ldr") == 0) {
        value = usb_send_cmd("GET_LDR", -1);
    }
    else if (strcmp(attr_name, "temp") == 0) {
        value = usb_send_cmd("GET_TEMP", -1);
    }
    else if (strcmp(attr_name, "hum") == 0) {
        value = usb_send_cmd("GET_HUM", -1);
    }

    sprintf(buff, "%d\n", value);                   // Cria a mensagem com o valor do led, ldr
    return strlen(buff);
}


// Essa função não deve ser alterada durante a task sysfs
// Executado quando o arquivo /sys/kernel/smartlamp/{led, ldr} é escrito (e.g., echo "100" | sudo tee -a /sys/kernel/smartlamp/led)
static ssize_t attr_store(struct kobject *sys_obj, struct kobj_attribute *attr, const char *buff, size_t count) {
    long ret, value =-1 , ret_write = -1;
    const char *attr_name = attr->attr.name;
    
    if (strcmp(attr_name, "ldr") == 0) {
    	printk(KERN_ALERT "Erro! Acesso ao arquivo não permitido\n");
    	return -EACCES;
    }

    if (strcmp(attr_name, "ldr") == 0) {
    	printk(KERN_ALERT "Erro! Acesso ao arquivo não permitido\n");
    	return -EACCES;
    }

    // Converte o valor recebido para long
    ret = kstrtol(buff, 10, &value);
    if (ret) {
        printk(KERN_ALERT "SmartLamp: valor de %s invalido.\n", attr_name);
        return -EACCES;
    }

    printk(KERN_INFO "SmartLamp: Setando %s para %ld ...\n", attr_name, value);

    if (ret < 0) {
        printk(KERN_ALERT "SmartLamp: erro ao setar o valor do %s.\n", attr_name);
        return -EACCES;
    }

    if (strcmp(attr_name, "led") == 0) {
    	ret_write = usb_send_cmd("SET_LED", value);
    	return strlen(buff);;
    }

    return strlen(buff);
}

static char* usb_write_serial(char *cmd, int param) {
    int ret, actual_size, retries;//,str_length; 
    char* resp_expected = (char*) kmalloc(MAX_RECV_LINE,GFP_KERNEL);  

    if(param != -1)sprintf(usb_out_buffer, "%s %d\n", cmd, param);
    else sprintf(usb_out_buffer, "%s\n", cmd);

    ret = usb_bulk_msg(smartlamp_device, usb_sndbulkpipe(smartlamp_device, usb_out), usb_out_buffer, strlen(usb_out_buffer), &actual_size, 1000*HZ);
    if (ret) {
        printk(KERN_ERR "SmartLamp: Erro de codigo %d ao enviar comando!\n", ret);
        return NULL;
    }
    sprintf(resp_expected, "RES %s ", cmd);
    return resp_expected; 
}

static int usb_read_serial(char* match) {
    int ret, actual_size;
    int retries = 10;                       // Tenta algumas vezes receber uma resposta da USB. Depois desiste.
    int j = 0;
    // Espera pela resposta correta do dispositivo (desiste depois de várias tentativas)
    while (retries > 0) {
        // Lê os dados da porta serial e armazena em usb_in_buffer
        // usb_in_buffer - contem a resposta em string do dispositivo
        // actual_size - contem o tamanho da resposta em bytes
        ret = usb_bulk_msg(smartlamp_device, usb_rcvbulkpipe(smartlamp_device, usb_in), usb_in_buffer, /*str_length*/ usb_max_size, &actual_size, 5000*HZ);

        if (ret) {
            printk(KERN_ERR "SmartLamp: Erro ao ler dados da USB (tentativa %d). Codigo: %d\n",retries--, ret);
            continue;
        }

        int i = 0;
       
        for (i = 0; i < actual_size; i++) {
            buffer[j++] = usb_in_buffer[i];
            if (usb_in_buffer[i] == '\n') {
                buffer[j] = '\0';
                int result = get_int_from_buffer(buffer, match);
                if (result != -9999)
                    return result;
                j = 0;
                buffer[j+1] = '\0';
            }
        }
    }

    return -1; 
}