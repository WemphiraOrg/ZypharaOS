#include "vbe.h"
#include "io.h"

static u16* const vga_buf = (u16*)0xB8000;
static int col=0, row=0;
static u8  cur_attr = 0x07;
static u32 vbe_fg = 0xFFFFFF, vbe_bg = 0x000000;
static int use_vbe = 0;

static const u32 vga_rgb[16] = {
    0x000000,0x0000AA,0x00AA00,0x00AAAA,
    0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
    0x555555,0x5555FF,0x55FF55,0x55FFFF,
    0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF,
};

void term_init(void) {
    use_vbe = vbe_current.active;
    col=row=0;
    cur_attr=0x07; vbe_fg=0xFFFFFF; vbe_bg=0x000000;
    if (use_vbe) vbe_fill(0x000000);
    else for(int i=0;i<80*25;i++) vga_buf[i]=(u16)' '|((u16)0x07<<8);
}

static int tcols(void){ return use_vbe ? vbe_cols() : 80; }
static int trows(void){ return use_vbe ? vbe_rows() : 25; }

static void scroll(void) {
    if (use_vbe) {
        u8* fb=(u8*)vbe_current.framebuffer;
        u32 row_bytes=vbe_current.pitch*16;
        u32 total=vbe_current.pitch*vbe_current.height;
        for(u32 i=0;i<total-row_bytes;i++) fb[i]=fb[i+row_bytes];
        for(u32 i=total-row_bytes;i<total;i++) fb[i]=0;
    } else {
        int c=tcols(),r=trows();
        for(int i=1;i<r;i++)
            for(int j=0;j<c;j++) vga_buf[(i-1)*c+j]=vga_buf[i*c+j];
        for(int j=0;j<c;j++) vga_buf[(trows()-1)*c+j]=(u16)' '|((u16)cur_attr<<8);
    }
}

void term_putchar(char ch) {
    int c=tcols(), r=trows();
    if(ch=='\n'){col=0;row++;}
    else if(ch=='\r'){col=0;}
    else if(ch=='\b'){
        if(col>0){col--;
            if(use_vbe) vbe_char(col*8,row*16,' ',vbe_fg,vbe_bg);
            else vga_buf[row*c+col]=(u16)' '|((u16)cur_attr<<8);
        }return;
    } else if(ch=='\t'){
        int ns=(col+8)&~7; while(col<ns&&col<c-1) term_putchar(' '); return;
    } else {
        if(use_vbe) vbe_char(col*8,row*16,ch,vbe_fg,vbe_bg);
        else vga_buf[row*c+col]=(u16)(u8)ch|((u16)cur_attr<<8);
        col++;
    }
    if(col>=c){col=0;row++;}
    if(row>=r){scroll();row=r-1;}
}

void term_print(const char* s){ while(*s) term_putchar(*s++); }

void term_print_int(int n) {
    if(n<0){term_putchar('-');n=-n;}
    if(n==0){term_putchar('0');return;}
    char b[12];int i=10;b[11]=0;
    while(n>0){b[i--]='0'+(n%10);n/=10;}
    term_print(b+i+1);
}

void term_print_uint(u32 n) {
    if(n==0){term_putchar('0');return;}
    char b[12];int i=10;b[11]=0;
    while(n>0){b[i--]='0'+(n%10);n/=10;}
    term_print(b+i+1);
}

void term_print_hex(u32 n) {
    term_print("0x");
    char b[9];b[8]=0;
    for(int i=7;i>=0;i--){int d=n&0xF;b[i]=d<10?'0'+d:'a'+d-10;n>>=4;}
    char*p=b;while(*p=='0'&&*(p+1))p++;
    term_print(p);
}

void term_setcolor(int fg, int bg) {
    if(use_vbe){ vbe_fg=vga_rgb[fg&15]; vbe_bg=vga_rgb[bg&15]; }
    else cur_attr=(u8)((bg<<4)|fg);
}

void term_clear(void) {
    if(use_vbe) vbe_fill(vbe_bg);
    else for(int i=0;i<80*25;i++) vga_buf[i]=(u16)' '|((u16)cur_attr<<8);
    col=row=0;
}

void term_gotoxy(int x,int y){ col=x; row=y; }
int  term_getcol(void){ return col; }
int  term_getrow(void){ return row; }
void term_puts_at(int x,int y,const char*s,int fg,int bg){
    int oc=col,or=row; col=x;row=y;
    int of=vbe_fg,ob=vbe_bg; u8 oa=cur_attr;
    term_setcolor(fg,bg);
    term_print(s);
    col=oc;row=or; vbe_fg=of;vbe_bg=ob;cur_attr=oa;
}
