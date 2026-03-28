#include "keyboard.h"
#include "idt.h"
#include "io.h"

#define KBD_DATA   0x60
#define KBD_STATUS 0x64
#define KBD_BUF    64

static volatile char kb_buf[KBD_BUF];
static volatile int  kb_head = 0;
static volatile int  kb_tail = 0;
static int shift = 0, ctrl = 0, alt = 0;

static const char sc_normal[128] = {
    0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', 8,  9,
    'q','w','e','r','t','y','u','i','o','p','[',']','\r', 0,'a','s',
    'd','f','g','h','j','k','l',';','\'','`', 0,'\\','z','x','c','v',
    'b','n','m',',','.','/', 0, '*', 0, ' ', 0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0, '7','8','9','-','4','5','6','+','1',
    '2','3','0','.', 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};
static const char sc_shift[128] = {
    0,  27,'!','@','#','$','%','^','&','*','(',')','_','+', 8,  9,
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\r', 0,'A','S',
    'D','F','G','H','J','K','L',':','"', '~', 0,'|', 'Z','X','C','V',
    'B','N','M','<','>','?', 0, '*', 0, ' ', 0,  0,  0,  0,  0,  0,
};

static void kb_push(char c) {
    int next = (kb_head + 1) % KBD_BUF;
    if (next != kb_tail) {
        kb_buf[kb_head] = c;
        kb_head = next;
    }
}

static void keyboard_irq(void) {
    u8 sc = inb(KBD_DATA);
    int released = sc & 0x80;
    sc &= 0x7F;

    /* Modificadores */
    if (sc == 0x2A || sc == 0x36) { shift = !released; return; }
    if (sc == 0x1D) { ctrl  = !released; return; }
    if (sc == 0x38) { alt   = !released; return; }

    if (released) return;

    /* ESC */
    if (sc == 0x01) { kb_push('\x1b'); return; }

    /* Flechas */
    if (sc == 0x48) { kb_push('\x80'); return; }  /* UP    */
    if (sc == 0x50) { kb_push('\x81'); return; }  /* DOWN  */
    if (sc == 0x4B) { kb_push('\x82'); return; }  /* LEFT  */
    if (sc == 0x4D) { kb_push('\x83'); return; }  /* RIGHT */

    /* F1-F10 -> \x90-\x99 */
    if (sc >= 0x3B && sc <= 0x44) { kb_push((char)(0x90 + sc - 0x3B)); return; }

    if (sc >= 128) return;
    char c = shift ? sc_shift[sc] : sc_normal[sc];
    if (!c) return;

    /* Ctrl+C, Ctrl+L etc */
    if (ctrl && c >= 'a' && c <= 'z') c -= 96;
    if (ctrl && c >= 'A' && c <= 'Z') c -= 64;

    kb_push(c);
}

void keyboard_init(void) {
    /* Flush buffer del teclado */
    while (inb(KBD_STATUS) & 0x01) inb(KBD_DATA);
    irq_set_handler(1, keyboard_irq);
}

int keyboard_has_key(void) { return kb_head != kb_tail; }

int keyboard_poll(void) {
    if (kb_head == kb_tail) return -1;
    char c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KBD_BUF;
    return (unsigned char)c;
}

char keyboard_getchar(void) {
    while (kb_head == kb_tail)
        __asm__ volatile("hlt");
    return (char)keyboard_poll();
}
