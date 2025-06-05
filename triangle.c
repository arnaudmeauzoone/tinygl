// gcc -o triangle triangle.c -lGL -lGLU -lglut -lm

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>


#define WIDTH 1200
#define HEIGHT 800

unsigned char *fb;
unsigned char *flip_buffer = NULL;


GLfloat angle = 0.0f; // Variable pour l'angle de rotation

// Fonction d'initialisation
void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0); // Définit la couleur de fond (noir)
    glEnable(GL_DEPTH_TEST); // Active le test de profondeur
}

// Remplacer gluLookAt() par cette implémentation manuelle
void setLookAt(float eyeX, float eyeY, float eyeZ,
               float centerX, float centerY, float centerZ,
               float upX, float upY, float upZ) {
    float forward[3], side[3], up[3];
    float matrix[16];
    
    // Calcul du vecteur forward (direction de vue)
    forward[0] = centerX - eyeX;
    forward[1] = centerY - eyeY;
    forward[2] = centerZ - eyeZ;
    float len = sqrt(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
    forward[0] /= len;
    forward[1] /= len;
    forward[2] /= len;
    
    // Normalisation du vecteur up
    len = sqrt(upX*upX + upY*upY + upZ*upZ);
    up[0] = upX / len;
    up[1] = upY / len;
    up[2] = upZ / len;
    
    // Calcul du vecteur side (cross product forward x up)
    side[0] = forward[1]*up[2] - forward[2]*up[1];
    side[1] = forward[2]*up[0] - forward[0]*up[2];
    side[2] = forward[0]*up[1] - forward[1]*up[0];
    
    // Recalcul du vrai up (cross product side x forward)
    up[0] = side[1]*forward[2] - side[2]*forward[1];
    up[1] = side[2]*forward[0] - side[0]*forward[2];
    up[2] = side[0]*forward[1] - side[1]*forward[0];
    
    // Construction de la matrice
    matrix[0] = side[0];
    matrix[4] = side[1];
    matrix[8] = side[2];
    matrix[12] = 0.0f;
    
    matrix[1] = up[0];
    matrix[5] = up[1];
    matrix[9] = up[2];
    matrix[13] = 0.0f;
    
    matrix[2] = -forward[0];
    matrix[6] = -forward[1];
    matrix[10] = -forward[2];
    matrix[14] = 0.0f;
    
    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    matrix[11] = 0.0f;
    matrix[15] = 1.0f;
    
    glMultMatrixf(matrix);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

// Fonction de rendu
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Efface le tampon de couleur et de profondeur
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // gluLookAt(0.0, 1.0, 5.0,
    //           0.0, 0.0, 0.0,
    //           0.0, 1.0, 0.0);

    //replace gluLookAt
    setLookAt(0.0, 1.0, 5.0,  // Position caméra
            0.0, 0.0, 0.0,  // Point visé
            0.0, 1.0, 0.0); // Vecteur up

    glRotatef(angle, 0.0f, 1.0f, 0.0f); // Applique une rotation autour de l'axe Y

    // Dessine une pyramide
    glBegin(GL_TRIANGLE_FAN);
        // Sommet de la pyramide
        glColor3f(1.0, 0.0, 0.0); // Rouge
        glVertex3f(0.0f, 1.0f, 0.0f);

        // Base de la pyramide
        glColor3f(0.0, 1.0, 0.0); // Vert
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glColor3f(0.0, 0.0, 1.0); // Bleu
        glVertex3f(1.0f, -1.0f, 1.0f);
        glColor3f(1.0, 1.0, 0.0); // Jaune
        glVertex3f(1.0f, -1.0f, -1.0f);
        glColor3f(1.0, 0.0, 1.0); // Magenta
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glColor3f(0.0, 1.0, 1.0); // Cyan
        glVertex3f(-1.0f, -1.0f, 1.0f);
    glEnd();


    glutSwapBuffers(); // Échange les tampons pour éviter le scintillement
    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, flip_buffer);
    
    // Inversion verticale
    for (int y = 0; y < HEIGHT; y++) {
        memcpy(fb + y * WIDTH * 4,
               flip_buffer + (HEIGHT - 1 - y) * WIDTH * 4,
               WIDTH * 4);
    }
}

// Fonction de mise à jour de l'animation
void update(int value) {
    angle += 2.0f; // Incrémente l'angle de rotation
    if (angle > 360) {
        angle -= 360;
    }
    glutPostRedisplay(); // Demande le réaffichage
    glutTimerFunc(16, update, 0); // Appelle à nouveau update après 16 ms (environ 60 FPS)
}

// Fonction de redimensionnement de la fenêtre
void reshape(int width, int height) {
    if (height == 0) height = 1; // Évite la division par zéro
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height); // Définit le viewport

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 100.0f); // Définit la projection perspective
}

// Fonction principale
int main(int argc, char** argv) {

    int fb_fd = open("/dev/shm/fb", O_RDWR | O_CREAT, 0666);
    ftruncate(fb_fd, WIDTH * HEIGHT * 4);
    fb = mmap(NULL, WIDTH*HEIGHT*4, PROT_READ|PROT_WRITE, MAP_SHARED, fb_fd, 0);
    flip_buffer = (unsigned char*)malloc(WIDTH * HEIGHT * 4);

    glutInit(&argc, argv); // Initialise GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // Active le double tampon et le test de profondeur
    glutInitWindowSize(1200, 800);
    glutCreateWindow("OpenGL Pyramid"); // Crée une fenêtre
    glutInitWindowPosition(100, 100);
    glutDisplayFunc(display); // Définit la fonction de rendu
    glutReshapeFunc(reshape); // Définit la fonction de redimensionnement
    init(); // Appelle la fonction d'initialisation
    glutTimerFunc(0, update, 0); // Démarre l'animation
    glutMainLoop(); // Entre dans la boucle principale de GLUT
    return 0;
}
