#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/ioport.h>
#include<asm/uaccess.h>
#include<asm/io.h>

MODULE_LICENSE("Dual BSD/GPL");

#define LCD(x) outb((c = ((0x0f & x) | (c & 0xf0) )), 0x378)
#define RS(x) outb((c = (((~(1 << 4)) & c) | (x << 4))), 0x378)
#define EN(x) outb((c = (((~(1 << 5)) & c) | (x << 5))), 0x378)
#define en 0b00010000
#define rs 0b00100000

#define lcd_space " "
#define device_name "LCD"
#define lcd_max 20 

static int lcd_open(struct inode *,struct file *);
static int lcd_release(struct inode *,struct file *);
static ssize_t lcd_read(struct file *,char __user *,size_t ,loff_t *);
static ssize_t lcd_write(struct file *,const char __user *,size_t ,loff_t *);

struct file_operations fops={
  .read=lcd_read,
  .write=lcd_write,
  .open=lcd_open,
  .release=lcd_release
};

int port;
int lcd_major=60;
static char lcd_buffer[lcd_max];
static int Device_open=0;
static unsigned char c;

int lcd_init(void);
void lcd_exit(void);

void lcd_setup(void);
void lcd_display(char *);
void lcd_clear(void);
void lcd_cmd(unsigned char );
void lcd_strobe(void);
void lcd_data(unsigned char );

int lcd_init(void) 
{
  int ret=register_chrdev(lcd_major,device_name,&fops);
  if(ret<0)
    {
      printk(KERN_INFO "Registered char device is failed with %d\n",ret);
      return ret;
}
  port=check_region(0x378,1);
  if(port)
    {
      printk(KERN_ALERT "Error in check_region port\n");
      ret=port;
      goto fail;
      return ret;
}
  request_region(0x378,1,"lcd");
  outb(0,0x378);
  lcd_setup();
  lcd_display("DRIVER INSET");
  return 0;
 fail:
  lcd_exit();
  return ret;
}

void lcd_exit(void)
{
  printk(KERN_ALERT "port is going to terminated\n");
  unregister_chrdev(lcd_major,device_name);
  lcd_display("DRIVER REMOVED");
}

static int lcd_open(struct inode *inode,struct file *filp)
{
  if(Device_open)
    return -EBUSY;
  Device_open++;
  return 0;
}

static int lcd_release(struct inode *inode,struct file *filp)
{
  Device_open--;
  return 0;
}

static ssize_t lcd_read(struct file *filp,char __user *buffer,size_t length,loff_t *offset)
{
  printk(KERN_ALERT "This operation is not supportd\n");
  return 0;
}

static ssize_t lcd_write(struct file *filp,const char __user *buffer,size_t length,loff_t *offset)
{
  static char buf[lcd_max];
  if(length<lcd_max)
    {
      copy_from_user(buf,buffer,length);
      buf[length]=0;
      lcd_display(buf);
      *offset+=length;
      return length;
	}
  else
    {
      copy_from_user(buf,buffer,lcd_max-1);
      buf[lcd_max-1]=0;
      lcd_display(buf);
      *offset+=lcd_max-1;
      return lcd_max-1;
}



}

void lcd_setup(void)
{
  lcd_clear();
  //  delay(300);
  lcd_cmd(0x38);
  // delay(300);
  lcd_cmd(0x0E);
}

void lcd_clear(void)
{
  lcd_cmd(0x01);
}

void lcd_cmd(unsigned char command)
{
  RS(0);
  LCD(command>>4);
  lcd_strobe();
  LCD(command);
  lcd_strobe();
}

void lcd_strobe(void)
{
  EN(1);
  udelay(100);
  EN(0);
  udelay(100);
}

void lcd_data(unsigned char data)
{
  RS(1);
  LCD(data>>4);
  lcd_strobe();
  LCD(data);
  lcd_strobe();
  RS(0);
}

void lcd_display(char *p)
{
  static int count=0;
  while(*p)
    {
      if(*p=='\n'||*p=='\0')
	{
      lcd_buffer[count++]=*p;
      lcd_data(*p);	
}else
	{
	  lcd_data(lcd_space);
	  lcd_buffer[count++]=lcd_space;
}
      p++;
 
}
  lcd_buffer[lcd_max]=0;
  msleep(30);
}
