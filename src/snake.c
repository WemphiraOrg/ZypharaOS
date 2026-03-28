/* Snake mejorado: usa interrupciones reales, sin lag */
#include "vga.h"
#include "vbe.h"

extern void  term_clear(void);
extern void  term_setcolor(int fg, int bg);
extern void  pit_sleep(u32 ms);
extern int   keyboard_poll(void);
extern int   keyboard_has_key(void);
extern u32   pit_ticks(void);

static u16* vga = (u16*)0xB8000;

static void vga_put(int x, int y, char c, int fg, int bg) {
    if (vbe_current.active) {
        /* Modo gráfico */
        extern void vbe_char(int,int,char,u32,u32);
        extern u32  vbe_rgb(u8,u8,u8);
        const u32 colors[16] = {
            0x000000,0x0000AA,0x00AA00,0x00AAAA,
            0xAA0000,0xAA00AA,0xAA5500,0xAAAAAA,
            0x555555,0x5555FF,0x55FF55,0x55FFFF,
            0xFF5555,0xFF55FF,0xFFFF55,0xFFFFFF
        };
        vbe_char(x*8, y*16, c, colors[fg&15], colors[bg&15]);
    } else {
        vga[y*80+x] = (u16)(u8)c | ((u16)((bg<<4)|fg) << 8);
    }
}

static void vga_str(int x, int y, const char* s, int fg, int bg) {
    while (*s) vga_put(x++, y, *s++, fg, bg);
}

static void vga_clr(int x, int y, int w, int fg, int bg) {
    for (int i = 0; i < w; i++) vga_put(x+i, y, ' ', fg, bg);
}

static void print_int_at(int x, int y, int n, int fg, int bg) {
    char buf[12]; int i=10; buf[11]=0;
    if (n==0){vga_put(x,y,'0',fg,bg);return;}
    while(n>0){buf[i--]='0'+(n%10);n/=10;}
    vga_str(x,y,buf+i+1,fg,bg);
}

#define BX 2
#define BY 2
#define BW 60
#define BH 22
#define MAX_LEN 300

static int sx[MAX_LEN], sy[MAX_LEN], slen;
static int dx, dy, fx, fy, score, hi_score;
static int paused, dead;

static unsigned int rng_s = 0xDEAD1234;
static unsigned int rng(void) {
    rng_s ^= rng_s<<13; rng_s ^= rng_s>>17; rng_s ^= rng_s<<5;
    return rng_s;
}

static void place_food(void) {
    int ok=0;
    while(!ok) {
        fx = BX+1+(int)(rng()%(BW-2));
        fy = BY+1+(int)(rng()%(BH-2));
        ok=1;
        for(int i=0;i<slen;i++) if(sx[i]==fx&&sy[i]==fy){ok=0;break;}
    }
}

static void draw_border(void) {
    /* Top */
    vga_put(BX, BY, '+', 11, 0);
    for(int x=1;x<BW-1;x++) vga_put(BX+x,BY,'-',11,0);
    vga_put(BX+BW-1,BY,'+',11,0);
    /* Sides */
    for(int y=1;y<BH-1;y++) {
        vga_put(BX,BY+y,'|',11,0);
        for(int x=1;x<BW-1;x++) vga_put(BX+x,BY+y,' ',7,0);
        vga_put(BX+BW-1,BY+y,'|',11,0);
    }
    /* Bottom */
    vga_put(BX,BY+BH-1,'+',11,0);
    for(int x=1;x<BW-1;x++) vga_put(BX+x,BY+BH-1,'-',11,0);
    vga_put(BX+BW-1,BY+BH-1,'+',11,0);

    /* Panel lateral */
    int px = BX+BW+2;
    vga_str(px, BY,    "+-----------+", 11, 0);
    vga_str(px, BY+1,  "|  S N A K E|", 14, 0);
    vga_str(px, BY+2,  "+-----------+", 11, 0);
    vga_str(px, BY+4,  "Score:", 7, 0);
    vga_str(px, BY+5,  "Best: ", 7, 0);
    vga_str(px, BY+7,  "W/A/S/D", 10, 0);
    vga_str(px, BY+8,  "mover", 7, 0);
    vga_str(px, BY+10, "ESC/Q", 10, 0);
    vga_str(px, BY+11, "salir", 7, 0);
    vga_str(px, BY+13, "SPACE", 10, 0);
    vga_str(px, BY+14, "pausa", 7, 0);
}

static void draw_score(void) {
    int px = BX+BW+2;
    vga_clr(px+6, BY+4, 6, 7, 0);
    vga_clr(px+6, BY+5, 6, 7, 0);
    print_int_at(px+6, BY+4, score, 14, 0);
    print_int_at(px+6, BY+5, hi_score, 10, 0);
}

static void draw_snake(void) {
    vga_put(fx, fy, '@', 12, 0);
    for(int i=1;i<slen;i++) vga_put(sx[i],sy[i],'o',10,0);
    vga_put(sx[0],sy[0],'O',15,0);
}

void snake_run(void) {
    /* Limpiar pantalla */
    if(vbe_current.active) {
        extern void vbe_fill(u32);
        vbe_fill(0x000000);
    } else {
        for(int i=0;i<80*25;i++) vga[i]=(u16)' '|((u16)(0<<4|7)<<8);
    }

    slen=4; dx=1; dy=0; score=0; paused=0; dead=0;
    int cx=BX+BW/2, cy=BY+BH/2;
    for(int i=0;i<slen;i++){sx[i]=cx-i;sy[i]=cy;}
    rng_s ^= pit_ticks();
    place_food();
    draw_border();
    draw_snake();
    draw_score();

    int speed=150, ndx=1, ndy=0;
    u32 last_tick = pit_ticks();

    while(!dead) {
        /* Procesar TODAS las teclas pendientes */
        int ch;
        while((ch = keyboard_poll()) != -1) {
            if(ch == '\x1b' || ch == 'q' || ch == 'Q') { dead=1; break; }
            if(ch == ' ' || ch == 'p') {
                paused ^= 1;
                int px=BX+BW/2-3;
                if(paused) vga_str(px,BY+BH/2,"PAUSA",14,0);
                else { for(int x=px;x<px+5;x++) vga_put(x,BY+BH/2,' ',7,0); }
            }
            if(!paused) {
                if((ch=='w'||ch=='\x80')&&dy!=1)  {ndx=0;ndy=-1;}
                if((ch=='s'||ch=='\x81')&&dy!=-1) {ndx=0;ndy=1;}
                if((ch=='a'||ch=='\x82')&&dx!=1)  {ndx=-1;ndy=0;}
                if((ch=='d'||ch=='\x83')&&dx!=-1) {ndx=1;ndy=0;}
            }
        }
        if(dead) break;
        if(paused) { pit_sleep(20); continue; }

        /* Mover solo cuando toca */
        u32 now = pit_ticks();
        u32 elapsed = (now - last_tick) * 10; /* ~10ms por tick a 100Hz */
        if(elapsed < (u32)speed) { pit_sleep(5); continue; }
        last_tick = now;

        /* Aplicar dirección */
        dx=ndx; dy=ndy;

        /* Borrar cola */
        vga_put(sx[slen-1],sy[slen-1],' ',7,0);

        /* Mover */
        for(int i=slen-1;i>0;i--){sx[i]=sx[i-1];sy[i]=sy[i-1];}
        sx[0]+=dx; sy[0]+=dy;

        /* Colisión con borde */
        if(sx[0]<=BX||sx[0]>=BX+BW-1||sy[0]<=BY||sy[0]>=BY+BH-1){dead=1;break;}

        /* Colisión consigo mismo */
        for(int i=1;i<slen;i++)
            if(sx[0]==sx[i]&&sy[0]==sy[i]){dead=1;break;}
        if(dead) break;

        /* Comer */
        if(sx[0]==fx&&sy[0]==fy) {
            score+=10;
            if(score>hi_score) hi_score=score;
            if(slen<MAX_LEN){sx[slen]=sx[slen-1];sy[slen]=sy[slen-1];slen++;}
            place_food();
            if(speed>60) speed-=3;
        }

        draw_snake();
        draw_score();
    }

    /* Game over */
    int gx=BX+BW/2-4, gy=BY+BH/2;
    vga_str(gx,   gy-1, "          ", 7, 0);
    vga_str(gx,   gy,   "GAME OVER!", 12, 0);
    vga_str(gx-2, gy+1, "Score: ", 7, 0);
    print_int_at(gx+5, gy+1, score, 14, 0);
    vga_str(gx-4, gy+3, "Pulsa cualquier tecla", 8, 0);

    /* Esperar tecla - con HLT para no quemar CPU */
    while(keyboard_poll() == -1)
        __asm__ volatile("hlt");
    /* Vaciar buffer */
    while(keyboard_poll() != -1);

    /* Limpiar pantalla al volver */
    if(vbe_current.active) {
        extern void vbe_fill(u32);
        vbe_fill(0x000000);
    } else {
        for(int i=0;i<80*25;i++) vga[i]=(u16)' '|((u16)(0<<4|7)<<8);
    }
}
