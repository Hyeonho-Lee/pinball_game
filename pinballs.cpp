#define _CRT_SECURE_NO_WARNINGS

#include <gl/glut.h>
#include <gl/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define   PI   3.1415926
#define M 36
#define N 18

float camera_theta, camera_phi, camera_radius;
float radius, theta, phi;

float ball_size;
float gravity, t, dt;
float ball_x, ball_y, ball_z;
float ball_vx, ball_vy, ball_vz;
float force, value_z, value_x;
float gnd_sx, gnd_sy, gnd_sz;
float side_sx, side_sy, side_sz;


int score;
int life;

bool game_start;

bool press_z, press_zz, press_x, press_xx, press_c;
GLuint   texture;

BITMAPINFOHEADER   bitmapInfoHeader;
unsigned char* bitmapImage;

unsigned char* LoadBitmapFile(const char* filename, BITMAPINFOHEADER* bitmapInfoHeader)
{
    FILE* filePtr;
    BITMAPFILEHEADER   bitmapFileHeader;
    unsigned char* bitmapImage;
    int               imageIdx = 0;
    unsigned char      tempRGB;

    filePtr = fopen(filename, "rb");
    if (filePtr == NULL)
        return NULL;

    fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

    if (bitmapFileHeader.bfType != 'MB') {
        fclose(filePtr);
        return   (NULL);
    }

    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

    fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

    if (!bitmapImage) {
        free(bitmapImage);
        fclose(filePtr);
        return NULL;
    }

    fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);

    if (bitmapImage == NULL) {
        fclose(filePtr);
        return NULL;
    }

    for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }

    fclose(filePtr);
    return   bitmapImage;
}

void init(void) {
    glEnable(GL_DEPTH_TEST);

    camera_theta = 11.0;
    camera_phi = 53.3;
    camera_radius = 16.0;

    radius = 1.0;
    theta = 0.0;
    phi = 0.0;

    ball_size = 0.25f;
    gravity = -9.81f;
    force = 3.0f;
    t = 0.0f;
    dt = 0.0005f;

    ball_x = 0.0f;
    ball_y = 5.0f;

    ball_vx = 0.1f;
    ball_vy = 0.1f;

    value_z = 0.0f;
    value_x = 0.0f;

    gnd_sx = 10.0f;
    gnd_sy = 0.4f;
    gnd_sz = 1.0f;

    side_sx = 0.4f;
    side_sy = 1.0f;
    side_sz = 14.0f;


    life = 3;
    //glEnable(GL_TEXTURE_2D);

    bitmapImage = LoadBitmapFile("Background.bmp", &bitmapInfoHeader);

    glGenTextures(10, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmapInfoHeader.biWidth, bitmapInfoHeader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmapImage);
    glShadeModel(GL_SMOOTH);
    printf("R 키를 눌러 게임을 시작하시오.\n");
    printf("z키: 왼쪽 슬라이드 / x키: 오른쪽 슬라이드\n");
    printf("방향키: 카메라 회전\n");
    printf("---------------------------------------------\n");
}

void DrawTextureCube(float tx, float ty, float tz)
{
    glPushMatrix();
    glTranslatef(tx, ty, tz);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(5.0f, -0.2f, 12.3f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-5.0f, -0.2f, 12.3f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-5.0f, -0.2f, -1.7f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(5.0f, -0.2f, -1.7f);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-5.0, 5.0, -5.0, 5.0, -5.0, 15.0);
    gluPerspective(60.0, 1.0, 1.0, 20.0);
}

void axis(void) {
    glBegin(GL_LINES);

    glColor3f(1.0, 0.0, 0.0); // x축 
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(100.0, 0.0, 0.0);

    glColor3f(0.0, 1.0, 0.0); // y축 
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 100.0, 0.0);

    glColor3f(0.0, 0.0, 1.0); // z축 
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 100.0);
    glEnd();
}

void frame_reset(void) {
    glClearColor(0.6, 0.6, 0.6, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void camera(void) {
    float   camera_x, camera_y, camera_z;

    camera_x = camera_radius * cos(camera_phi) * cos(camera_theta);
    camera_y = camera_radius * cos(camera_phi) * sin(camera_theta);
    camera_z = camera_radius * sin(camera_phi);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camera_x, camera_y, camera_z, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
}

void ground(float x, float y, float z) {
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(x, y, z);
    glScalef(5.0f, 5.0f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void Wall(float x, float y, float z, float sx, float sy, float sz) {
    glPushMatrix();
    glColor3f(1.0f, 0.5f, 0.2f);
    glTranslatef(x, y, z);
    glScalef(sx, sy, sz);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void Attack_Left(float x, float y, float z) {
    glPushMatrix();
    if (press_z == true) {
        if (value_z < 30.0f) {
            value_z += 20.0f;
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        glTranslatef(x, y, z + 0.5f);
        glRotatef(value_z, 0.0f, 1.0f, 0.0f);
        glScalef(3.5f, 1.0f, 0.5f);
        glutSolidCube(1.0f);

    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
        glTranslatef(x, y, z);
        glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
        glScalef(3.5f, 1.0f, 0.5f);
        glutSolidCube(1.0f);
    }

    if ((x + 2.0f >= ball_x && x - 1.25f <= ball_x) && (z + 0.5f >= ball_y && z - 0.5f <= ball_y)) {
        if (press_zz == true) {
            ball_vx = -0.2f;
            ball_vy = 0.3f;
        }
    }

    if ((x + 2.0f >= ball_x && x - 1.25f <= ball_x) && (z + 0.2f >= ball_y && z - 0.2f <= ball_y)) {
        if (press_zz == false) {
            ball_vy = -ball_vy;
        }
    }
    glPopMatrix();
}

void Attack_Right(float x, float y, float z) {
    glPushMatrix();
    if (press_x == true) {
        if (value_x > -30.0f) {
            value_x -= 20.0f;
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        glTranslatef(x, y, z + 0.5f);
        glRotatef(value_x, 0.0f, 1.0f, 0.0f);
        glScalef(3.5f, 1.0f, 0.5f);
        glutSolidCube(1.0f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
        glTranslatef(x, y, z);
        glRotatef(0.0f, 0.0f, 1.0f, 0.0f);
        glScalef(3.5f, 1.0f, 0.5f);
        glutSolidCube(1.0f);
    }

    if ((x + 1.25f >= ball_x && x - 2.0f <= ball_x) && (z + 0.5f >= ball_y && z - 0.5f <= ball_y)) {
        if (press_xx == true) {
            ball_vx = 0.2f;
            ball_vy = 0.3f;
        }
    }

    if ((x + 1.25f >= ball_x && x - 2.0f <= ball_x) && (z + 0.2f >= ball_y && z - 0.2f <= ball_y)) {
        if (press_xx == false) {
            ball_vy = -ball_vy;
        }
    }
    glPopMatrix();
}

void damage() {
    score = score + 1;
    printf("점수: %d\n", score);
}

void Enemy() {
    glPushMatrix();
    glColor3f(0.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, 4.0f);
    glRotatef(0.0f, 0.0f, 0.0f, 0.0f);
    glScalef(1.0f, 1.0f, 1.0f);
    glutSolidSphere(0.5f, 20.0f, 20.0f);
    glPopMatrix();

    if (ball_x <= 0.5f && ball_x >= 0.3f && ball_y <= 4.5f && ball_y >= 3.5f) {
        ball_vx = -ball_vx;
        damage();
    }

    if (ball_x >= -0.5f && ball_x <= -0.3f && ball_y <= 4.5f && ball_y >= 3.5f) {
        ball_vx = -ball_vx;
        damage();
    }

    if (ball_x <= 0.5f && ball_x >= -0.5f && ball_y <= 3.5f && ball_y >= 3.2f) {
        ball_vy = -ball_vy;
        damage();
    }

    if (ball_x <= 0.5f && ball_x >= -0.5f && ball_y >= 4.2f && ball_y <= 4.5f) {
        ball_vy = -ball_vy;
        damage();
    }
}

void die() {
    if (game_start) {

        if (life <= 0) {
            game_start = false;
            printf("게임오버!\n");
        }


        if (ball_y <= -0.2f) {
            ball_x = 0.0f;
            ball_y = 5.0f;

            ball_vx = 0.1f;
            ball_vy = 0.1f;
            life = life - 1;
            printf("\n목숨: %d\n\n", life);
        }
    }
}

void display(void) {
    frame_reset();

    camera();
    axis();

    Attack_Left(2.5, 0, 0);
    Attack_Right(-2.5, 0, 0);

    //DrawTextureCube(0.0f, 0.0f, 0.0f);
    Wall(0.0, 0.0, -1.2, gnd_sx, gnd_sz, gnd_sy);  //바닥
    Wall(-4.6, 0.0, 5.3, side_sx, side_sy, side_sz);    //left wall
    Wall(4.6, 0.0, 5.3, side_sx, side_sy, side_sz);     //right wall
    Wall(0.0, 0.0, 12.4, gnd_sx, gnd_sz, gnd_sy);   //천장

    if (game_start) {
        glPushMatrix();
        glColor3f(0.0f, 1.0f, 0.0f);
        glTranslatef(ball_x, ball_z, ball_y);
        glScalef(1.0f, 1.0f, 1.0f);
        glutSolidSphere(ball_size, 100, 100);
        glPopMatrix();

        Enemy();
    }

    die();

    glFlush();
    glutSwapBuffers();
}

void special_key(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_DOWN:
        camera_phi -= 0.1;
        //printf("%f\n", camera_phi);
        break;
    case GLUT_KEY_UP:
        camera_phi += 0.1;
        //printf("%f\n", camera_phi);
        break;
    case GLUT_KEY_LEFT:
        camera_theta -= 0.1;
        //printf("%f\n", camera_theta);
        break;
    case GLUT_KEY_RIGHT:
        camera_theta += 0.1;
        //printf("%f\n", camera_theta);
        break;
    default: break;
    }
    glutPostRedisplay();
}


void my_key_down(unsigned char key, int x, int y) {
    switch (key) {
    case 'a':      camera_radius += 0.1;      break;
    case 's':      camera_radius -= 0.1;      break;
    case 'z':
        press_z = true;
        press_zz = true;
        break;
    case 'x':
        press_x = true;
        press_xx = true;
        break;
    case 'c':
        press_c = true;
        break;
    case 'r':
        game_start = true;
        break;
    default: break;
    }
    glutPostRedisplay();
}

void my_key_up(unsigned char key, int x, int y) {
    switch (key) {
    case 'z':
        press_z = false;
        press_zz = false;
        value_z = 0.0f;
        break;
    case 'x':
        press_x = false;
        press_xx = false;
        value_x = 0.0f;
        break;
    case 'c':
        press_c = false;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void Timer(int Value) {
    if (game_start) {
        ball_vy += gravity * 2.0f * dt;
        ball_y += ball_vy + 0.5f * gravity * dt * dt;

        ball_x += ball_vx + 0.5f * force * dt * dt;

        if (ball_y <= -0.5f || ball_y >= 6.0f) {
            ball_vy = -ball_vy;
        }

        if (ball_x <= -3.8f) {
            ball_vx = -ball_vx;
        }

        if (ball_x >= 3.8f) {
            ball_vx = -ball_vx;
        }

        if (ball_x >= 0) {
            ball_vx += 0.001f;
        }

        if (ball_x <= 0) {
            ball_vx -= 0.001f;
        }
    }

    glutPostRedisplay();
    if (press_c == true) {
        glutTimerFunc(500, Timer, 1);
    }
    else {
        glutTimerFunc(60, Timer, 1);
    }
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Pin_Ball");
    init();
    glutTimerFunc(50, Timer, 1);
    glutDisplayFunc(display);
    glutSpecialFunc(special_key);
    glutKeyboardFunc(my_key_down);
    glutKeyboardUpFunc(my_key_up);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}
