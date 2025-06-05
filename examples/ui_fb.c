#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "../src/zgl.h"

typedef struct {
    GLContext *gl_context;
    int xsize, ysize;
    int do_convert;
    void *framebuffer;
} TinyGLContext;

/* Interface minimale requise */
void draw(void);
void idle(void);
void init(void);
void reshape(int width, int height);

void tkSwapBuffers(void)
{
    GLContext *gl_context = gl_get_context();
    TinyGLContext *ctx = (TinyGLContext *)gl_context->opaque;
    
    if (ctx->do_convert) {
        ZB_copyFrameBuffer(gl_context->zb, ctx->framebuffer, ctx->xsize * 4);
    }
    // Pas besoin de copie si pas de conversion, car le Z-Buffer écrit directement
}

int ui_loop(int argc, char **argv, const char *name)
{
    TinyGLContext ctx;
    int width = 1200;
    int height = 800;
    
    // Initialisation du framebuffer
    size_t fb_size = width * height * 4;
    int fb_fd = open("/dev/shm/fb", O_RDWR | O_CREAT, 0666);
    if (fb_fd == -1) {
        perror("Failed to open /dev/shm/fb");
        return 1;
    }
    
    if (ftruncate(fb_fd, fb_size) == -1) {
        perror("Failed to resize /dev/shm/fb");
        close(fb_fd);
        return 1;
    }
    
    void *fb_mmap = mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if (fb_mmap == MAP_FAILED) {
        perror("Failed to mmap /dev/shm/fb");
        close(fb_fd);
        return 1;
    }
    
    // Configuration du contexte
    ctx.xsize = width;
    ctx.ysize = height;
    ctx.framebuffer = fb_mmap;
    
    // Initialisation de TinyGL
    int mode = ZB_MODE_RGBA; // Mode 32-bit RGBA
    ctx.do_convert = (TGL_FEATURE_RENDER_BITS != 32);
    
    ZBuffer *zb = ZB_open(width, height, mode, 0, NULL, NULL, ctx.do_convert ? NULL : fb_mmap);
    if (!zb) {
        fprintf(stderr, "Failed to initialize Z buffer\n");
        return 1;
    }
    
    glInit(zb);
    ctx.gl_context = gl_get_context();
    ctx.gl_context->opaque = &ctx;
    
    // Initialisation utilisateur
    init();
    reshape(width, height);
    
    // Boucle principale simplifiée
    while (1) {
        idle();
    }
    
    // Nettoyage (en théorie jamais atteint)
    munmap(fb_mmap, fb_size);
    close(fb_fd);
    return 0;
}