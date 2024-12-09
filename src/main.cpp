// Tamaño de la pantalla 240x320 píxeles

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Sprite.h"

#define TFT_DC 7
#define TFT_CS 6
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_RST 10
#define TFT_MISO 12

#define botonRight 18
#define botonLeft 19
#define botonUp 20
#define botonDown 21
#define botonX 17
#define pinTone 16

const int XMAX = 240; // Ancho del display
const int YMAX = 320; // Alto del display
int x = 120; // Posición x del jugador
int y = 150; // Posición y del jugador
const uint8_t UP = 0;
const uint8_t DOWN = 1;
const uint8_t RIGHT = 2;
const uint8_t LEFT = 3;
int currentFrame = 2;
int direction = UP; // Dirección inicial de la nave

#define ASTEROID_WIDTH 30
#define ASTEROID_HEIGHT 25
#define BULLET_WIDTH 2
#define BULLET_HEIGHT 3

struct Asteroid {
  int x, y;
  int speedX, speedY;
  uint16_t color;
};

struct Bullet {
  int x, y;
  int speedX, speedY;
  bool active;
};

#define MAX_BULLETS 5
Bullet bullets[MAX_BULLETS];

Adafruit_ILI9341 screen = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

#define MAX_ASTEROIDS 10
Asteroid asteroids[MAX_ASTEROIDS];

void setPlayerPosition(int x, int y);
void moverPlayer(uint8_t direccion);
void moverPlayerDerecha(void);
void moverPlayerIzquierda(void);
void moverPlayerArriba(void);
void moverPlayerAbajo(void);
void generarAsteroides(void);
void moverAsteroides(void);
void dibujarAsteroides(void);
void disparar(void);
void moverBullet(void);
void dibujarBullet(void);
void detectarColisiones(void); 
void detectarColisionNaveAsteroide(void);
void mostrarPantallaPerdiste(void);
void reiniciarJuego(void);
bool todosAsteroidesDestruidos();
void mostrarPantallaGanaste(void);


void setup() {
    pinMode(pinTone,OUTPUT);
    Serial.begin(9600);
    Serial.println("Serial inicializado");

    screen.begin();
    screen.fillScreen(ILI9341_BLACK);

    generarAsteroides();
}

void loop() {
    // Comprobamos el estado de los botones y movemos la nave si están presionados
    if(digitalRead(botonRight) == HIGH) {
        moverPlayer(RIGHT);
        currentFrame = 1;
    }
    if(digitalRead(botonLeft) == HIGH) {
        moverPlayer(LEFT);
        currentFrame = 0;
    }
    if(digitalRead(botonUp) == HIGH) {
        moverPlayer(UP);
        currentFrame = 2;
    }
    if(digitalRead(botonDown) == HIGH) {
        moverPlayer(DOWN);
        currentFrame = 3;
    }

    // Comprobamos el estado del botón X para disparar
    if(digitalRead(botonX) == HIGH) {
        disparar(); // Dispara si el botón X está presionado
    }

    // Mover y dibujar el proyectil
    moverBullet();
    dibujarBullet();

    // Detectar colisiones entre las balas y los asteroides
    detectarColisiones();

    // Detectar colisión entre nave y asteroides
    detectarColisionNaveAsteroide();

    if (todosAsteroidesDestruidos()) {
    mostrarPantallaGanaste();  // Muestra la pantalla de victoria
    }

    // Animar el jugador
    setPlayerPosition(x, y);
    
    // Mover asteroides
    moverAsteroides();
    
    // Dibujar asteroides
    dibujarAsteroides();
}

void setPlayerPosition(int x1, int y1) {
  x = x1;
  y = y1;
  screen.fillRect(x, y, 36, 36, ILI9341_BLACK);
  screen.drawBitmap(x, y, xWing[currentFrame], 36, 36, ILI9341_RED);
}

void moverPlayer(uint8_t newDirection) {
    direction = newDirection;
    uint8_t delta = 2;
    
    switch (direction)
    {
    case UP:
        if (y > 0) { 
            y = y - delta;
        }
        break;
        
    case DOWN:
        if (y < YMAX - 36) { 
            y = y + delta;
        }
        break;
        
    case RIGHT:
        if (x < XMAX - 36) { 
            x = x + delta;
        } else {
            x = XMAX - 36; 
        }
        break;
        
    case LEFT:
        if (x > 0) { 
            x = x - delta;
        }
        break;
    }
}

void generarAsteroides() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    asteroids[i].speedX = random(1, 3) * (random(0, 2) == 0 ? 1 : -1); 
    asteroids[i].speedY = random(1, 3) * (random(0, 2) == 0 ? 1 : -1);  

    int edge = random(0, 4);
    if (edge == 0) {
      asteroids[i].x = random(0, XMAX - ASTEROID_WIDTH); 
      asteroids[i].y = 0;
    } else if (edge == 1) {
      asteroids[i].x = random(0, XMAX - ASTEROID_WIDTH);  
      asteroids[i].y = YMAX - ASTEROID_HEIGHT;
    } else if (edge == 2) {
      asteroids[i].x = 0;  
      asteroids[i].y = random(0, YMAX - ASTEROID_HEIGHT);
    } else {
      asteroids[i].x = XMAX - ASTEROID_WIDTH;  
      asteroids[i].y = random(0, YMAX - ASTEROID_HEIGHT);
    }

    asteroids[i].color = ILI9341_YELLOW; 
  }
}

void moverAsteroides() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    asteroids[i].x += asteroids[i].speedX;
    asteroids[i].y += asteroids[i].speedY;

    if (asteroids[i].x <= 0 || asteroids[i].x >= XMAX - ASTEROID_WIDTH) {
      asteroids[i].speedX = -asteroids[i].speedX;  
    }

    if (asteroids[i].y <= 0 || asteroids[i].y >= YMAX - ASTEROID_HEIGHT) {
      asteroids[i].speedY = -asteroids[i].speedY;  
    }

    for (int j = i + 1; j < MAX_ASTEROIDS; j++) {
      if (asteroids[i].x < asteroids[j].x + ASTEROID_WIDTH &&
          asteroids[i].x + ASTEROID_WIDTH > asteroids[j].x &&
          asteroids[i].y < asteroids[j].y + ASTEROID_HEIGHT &&
          asteroids[i].y + ASTEROID_HEIGHT > asteroids[j].y) {
        
        int tempSpeedX = asteroids[i].speedX;
        int tempSpeedY = asteroids[i].speedY;

        asteroids[i].speedX = asteroids[j].speedX;
        asteroids[i].speedY = asteroids[j].speedY;

        asteroids[j].speedX = tempSpeedX;
        asteroids[j].speedY = tempSpeedY;
      }
    }
  }
}

void dibujarAsteroides() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    screen.fillRect(asteroids[i].x, asteroids[i].y, ASTEROID_WIDTH, ASTEROID_HEIGHT, ILI9341_BLACK);  
    screen.drawBitmap(asteroids[i].x, asteroids[i].y, astd, ASTEROID_WIDTH, ASTEROID_HEIGHT, asteroids[i].color);
  }
}

void disparar() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = x + 18;
      bullets[i].y = y + 18;
      bullets[i].active = true;
      tone(pinTone,2000,300);

      if (direction == UP) {
        bullets[i].speedX = 0;
        bullets[i].speedY = -8;
        bullets[i].x = x + 18;
      } else if (direction == DOWN) {
        bullets[i].speedX = 0;
        bullets[i].speedY = 8;
        bullets[i].x = x + 18;
        bullets[i].y = y + 36;
      } else if (direction == RIGHT) {
        bullets[i].speedX = 8;
        bullets[i].speedY = 0;
        bullets[i].y = y + 18;
        bullets[i].x = x + 36;
      } else if (direction == LEFT) {
        bullets[i].speedX = -8;
        bullets[i].speedY = 0;
        bullets[i].y = y + 18;
        bullets[i].x = x;
      }

      break;
    }
  }
}

void moverBullet() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
        screen.fillRect(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, ILI9341_BLACK);
      bullets[i].x += bullets[i].speedX;
      bullets[i].y += bullets[i].speedY;

      if (bullets[i].x < 0 || bullets[i].x > XMAX || bullets[i].y < 0 || bullets[i].y > YMAX) {
        bullets[i].active = false;
      }
    }
  }
}

void dibujarBullet() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      screen.fillRect(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, ILI9341_WHITE);
    }
  }
}
void detectarColisiones() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      for (int j = 0; j < MAX_ASTEROIDS; j++) {
        // Detectar colisión entre bala y asteroide
        if (asteroids[j].x < bullets[i].x + BULLET_WIDTH &&
            asteroids[j].x + ASTEROID_WIDTH > bullets[i].x &&
            asteroids[j].y < bullets[i].y + BULLET_HEIGHT &&
            asteroids[j].y + ASTEROID_HEIGHT > bullets[i].y) {
              
          // Borrar asteroide y bala de la pantalla
          tone(pinTone,200,500);
          screen.fillRect(asteroids[j].x, asteroids[j].y, ASTEROID_WIDTH, ASTEROID_HEIGHT, ILI9341_BLACK);
          screen.fillRect(bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, ILI9341_BLACK);
          
          // Desactivar la bala
          bullets[i].active = false;

          // Mover el asteroide fuera de la pantalla para indicar que ya no está activo
          asteroids[j].x = -100;  // Posición fuera de los límites visibles
          asteroids[j].y = -100;

          break;  // Salir del bucle interno ya que la bala solo puede colisionar con un asteroide
        }
      }
    }
  }
}

void detectarColisionNaveAsteroide() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    // Detectar colisión entre la nave y un asteroide
    if (asteroids[i].x < x + 36 && 
        asteroids[i].x + ASTEROID_WIDTH > x &&
        asteroids[i].y < y + 36 &&
        asteroids[i].y + ASTEROID_HEIGHT > y) {
      
      mostrarPantallaPerdiste();
    }
  }
}
void mostrarPantallaPerdiste() {
  // Borrar toda la pantalla
  screen.fillScreen(ILI9341_BLACK);

  // Mostrar mensaje de "Perdiste"
  screen.setTextColor(ILI9341_RED);
  screen.setTextSize(3);
  screen.setCursor(50, YMAX / 2 - 30);  // Centrar mensaje principal
  screen.print("Perdiste");

  // Mostrar mensaje de "¿Reintentar?"
  screen.setTextColor(ILI9341_WHITE);
  screen.setTextSize(2);
  screen.setCursor(50, YMAX / 2 + 20);  // Mensaje más pequeño, un poco abajo
  screen.print("Reintentar?");
  tone(pinTone,800,200);
  delay(250);
  tone(pinTone,600,200);
  delay(250);
  tone(pinTone,400,300);
  delay(350);
  tone(pinTone,200,500);
  delay(550);
  // Esperar interacción del usuario
  while (true) {
    if (digitalRead(botonX) == HIGH) {
      reiniciarJuego();  // Reinicia el juego cuando se presiona el botón X
      break;  // Sale del bucle infinito
    }
  }
}
void reiniciarJuego() {
  // Reiniciar variables del jugador
  x = 120;
  y = 150;
  direction = UP;
  currentFrame = 0;

  // Reiniciar asteroides
  generarAsteroides();

  // Reiniciar balas
  for (int i = 0; i < MAX_BULLETS; i++) {
    bullets[i].active = false;
  }

  // Borrar la pantalla
  screen.fillScreen(ILI9341_BLACK);

  // Redibujar el jugador
  setPlayerPosition(x, y);
}
bool todosAsteroidesDestruidos() {
  for (int i = 0; i < MAX_ASTEROIDS; i++) {
    if (asteroids[i].x >= 0 && asteroids[i].x <= XMAX &&
        asteroids[i].y >= 0 && asteroids[i].y <= YMAX) {
      return false;  // Si al menos un asteroide está activo, no han sido destruidos todos
    }
  }
  return true;  // Todos los asteroides están fuera de la pantalla
}

void mostrarPantallaGanaste() {
  // Borrar toda la pantalla
  screen.fillScreen(ILI9341_BLACK);

  // Mostrar mensaje de "Ganaste"
  screen.setTextColor(ILI9341_GREEN);
  screen.setTextSize(3);
  screen.setCursor(50, YMAX / 2 - 30);  // Centrar mensaje principal
  screen.print("Ganaste");

  // Mostrar mensaje de "Volver a jugar"
  screen.setTextColor(ILI9341_WHITE);
  screen.setTextSize(2);
  screen.setCursor(50, YMAX / 2 + 20);  // Mensaje más pequeño, un poco abajo
  screen.print("Volver a jugar?");

  tone(pinTone,200,200);
  delay(250);
  tone(pinTone,400,200);
  delay(250);
  tone(pinTone,600,300);
  delay(350);
  tone(pinTone,800,500);
  delay(550);

  // Esperar interacción del usuario
  while (true) {
    if (digitalRead(botonX) == HIGH) {
      reiniciarJuego();  // Reinicia el juego cuando se presiona el botón X
      break;  // Sale del bucle infinito
    }
  }
}

