#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

float A, B, C;

float cubeWidth = 20;
float cubeWidthX = 20;
float cubeWidthY = 20;
int width = 160, height = 44;
float zBuffer[160 * 44];
char buffer[160 * 44];
int backgroundASCIICode = ' ';
int distanceFromCam = 150;
float horizontalOffset = 40;
float K1 = 40;
float cubeZ = 5;

float incrementSpeed = 0.4;
float incrementArm = 0.4;

float a_lengh = 40, b_lengh = 40; // dlugosci ramienia

float x, y, z;
float ooz;
int xp, yp;
int idx;

// Rotation matrixex for joint 1
float calculateX1(float i, float j, float k, float a) {
  return i * cos(A) + a * cos(A) * cos(B) - j * sin(A);
}

float calculateZ1(float i, float j, float k, float a) {
  return i * sin(A) + j * cos(A) + a * sin(A) * cos(B);
}

float calculateY1(float i, float j, float k, float a) {
  return k + a * sin(B);
}

// Projection on screen
void calculateForSurface1(float cubeX, float cubeY, float cubeZ, float a, int ch) {
  x = calculateX1(cubeX, cubeY, cubeZ, a);
  y = calculateY1(cubeX, cubeY, cubeZ, a);
  z = calculateZ1(cubeX, cubeY, cubeZ, a) + distanceFromCam;

  ooz = 1 / z;

  xp = (int)(width * 3 / 4 + horizontalOffset + K1 * ooz * x * 2);
  yp = (int)(height * 3 / 4  + K1 * ooz * y);

  idx = xp + yp * width;
  if (idx >= 0 && idx < width * height) {
    if (ooz > zBuffer[idx]) {
      zBuffer[idx] = ooz;
      buffer[idx] = ch;
    }
  }
}

// Rotation matrixex for joint 2
float calculateX2(float i, float j, float k, float b) {
  return a_lengh * cos(A) * cos(B) + b * cos(A) * cos(B + C) + i * cos(A) - j * sin(A);
}

float calculateZ2(float i, float j, float k, float b) {
  return a_lengh * cos(A) * cos(B) + b * cos(A) * cos(B + C) + i * sin(A) + j * cos(A);
}

float calculateY2(float i, float j, float k, float b) {
  return k + a_lengh * sin(B) + b * sin(B + C);
}

// Projection on screen
void calculateForSurface2(float cubeX, float cubeY, float cubeZ, float b, int ch) {
  x = calculateX2(cubeX, cubeY, cubeZ, b);
  y = calculateY2(cubeX, cubeY, cubeZ, b);
  z = calculateZ2(cubeX, cubeY, cubeZ, b) + distanceFromCam;

  ooz = 1 / z;

  xp = (int)(width * 3 / 4 + horizontalOffset + K1 * ooz * x * 2);
  yp = (int)(height * 3 / 4 + K1 * ooz * y);

  idx = xp + yp * width;
  if (idx >= 0 && idx < width * height) {
    if (ooz > zBuffer[idx]) {
      zBuffer[idx] = ooz;
      buffer[idx] = ch;
    }
  }
}

void display() {

  memset(buffer, backgroundASCIICode, width * height);
  memset(zBuffer, 0, width * height * 4);
  cubeWidthX = 4;
  cubeWidthY = -4;
  cubeZ = 4;
  horizontalOffset = 0; //cubeWidth;

  // Joint 1
  for (float cubeX = -cubeWidthX; cubeX < cubeWidthX; cubeX += incrementSpeed) {
    for (float cubeY = 0; cubeY > cubeWidthY; cubeY -= incrementSpeed) { // odwrócowne bo wzpółrzeędne są odwrócone
      for (float a = 0; a <= a_lengh; a += incrementArm) {

        calculateForSurface1(cubeX, cubeY, cubeZ, a, '@');
        calculateForSurface1(cubeZ, cubeY, cubeX, a, '+');
        calculateForSurface1(-cubeZ, cubeY, -cubeX, a, '~');
        calculateForSurface1(-cubeX, cubeY, -cubeZ, a, '.');
        //calculateForSurface(cubeX, 0, cubeZ, a, '$');
        //calculateForSurface(cubeX, cubeWidthY, cubeZ, a, '+');
      }
    }
  }

  // Joint 2
  for (float cubeX = -cubeWidthX; cubeX < cubeWidthX; cubeX += incrementSpeed) {
    for (float cubeY = 0; cubeY > cubeWidthY; cubeY -= incrementSpeed) { // odwrócowne bo wzpółrzeędne są odwrócone
      for (float b = 0; b <= b_lengh; b += incrementArm) {

        calculateForSurface2(cubeX, cubeY, cubeZ, b, '@');
        calculateForSurface2(cubeZ, cubeY, cubeX, b, '+');
        calculateForSurface2(-cubeZ, cubeY, -cubeX, b, '~');
        calculateForSurface2(-cubeX, cubeY, -cubeZ, b, '.');
        //calculateForSurface(cubeX, 0, cubeZ, b, '$');
        //calculateForSurface(cubeX, cubeWidthY, cubeZ, b, '+');
      }
    }
  }

  printf("\x1b[H");
  for (int k = 0; k < width * height; k++) {
    putchar(k % width ? buffer[k] : 10);
  }
}

// ---------- Terminal display stuff ------------


// Function to set terminal to non-canonical mode
void set_terminal_mode(struct termios *original_termios) {
  struct termios new_termios;

  // Get the current terminal settings
  tcgetattr(STDIN_FILENO, original_termios);

  // Copy the current terminal settings to modify them
  new_termios = *original_termios;

  // Disable canonical mode and echo
  new_termios.c_lflag &= ~(ICANON | ECHO);

  // Set the minimum number of characters for noncanonical read
  new_termios.c_cc[VMIN] = 1;
  new_termios.c_cc[VTIME] = 0;

  // Set the new terminal settings immediately
  tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

// Function to reset terminal to the original mode
void reset_terminal_mode(const struct termios *original_termios) {
  tcsetattr(STDIN_FILENO, TCSANOW, original_termios);
}

// Function to set the file descriptor to non-blocking mode
void set_nonblocking_mode(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
  struct termios original_termios;

  // Set the terminal to non-canonical mode
  set_terminal_mode(&original_termios);

  // Set the stdin to non-blocking mode
  set_nonblocking_mode(STDIN_FILENO);

  char ch;
  int running = 1;
  while (running) {
    // Read a character from stdin
    ssize_t n = read(STDIN_FILENO, &ch, 1);
    if (n > 0) {
      if (ch == 'z') {
        running = 0;
      }
      if (ch == 'e') {
        A += 0.05;
      }
      if (ch == 'q') {
        A -= 0.05;
      }
      if (ch == 'a') {
        B += 0.05;
      }
      if (ch == 'd') {
        B -= 0.05;
      }
      if (ch == 'w') {
        C += 0.05;
      }
      if (ch == 's') {
        C -= 0.05;
      }
    }


    display();


    // Sleep for 10 milliseconds
    usleep(3000);
  }

  // Reset the terminal to the original mode
  reset_terminal_mode(&original_termios);
  printf("\n");

  return 0;
}