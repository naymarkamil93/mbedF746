#include <mbed.h>
#include <threadLvgl.h>
#include "demos/lv_demos.h"
#include <cstdio>

//broches
const PinName BUTTON_UP = D0; //Broche joystick haut
const PinName BUTTON_DOWN = D1; // Broche joystick bas
const PinName LED_PIN = D2; // Broche de la LED


ThreadLvgl threadLvgl(30);  // Thread LVGL avec un taux de rafraîchissement de 30ms


const int SCREEN_WIDTH = 480;   // Largeur de l'écran
const int SCREEN_HEIGHT = 272;  // Hauteur de l'écran


// Propriétés de la raquette
const int PADDLE_WIDTH = 10;     //Epaisseur  de la raquette
const int PADDLE_HEIGHT = 50;   // Hauteur de la raquette
const int PADDLE_SPEED = 5;     // Vitesse de déplacement de la raquette

// Propriétés de la balle
const int BALL_SIZE = 10;        // Taille de la balle
const int BALL_SPEED_X = 4;     // Vitesse de déplacement de la balle en direction X
const int BALL_SPEED_Y = 4;     // Vitesse de déplacement de la balle en direction Y

// Creation des objets LVGL pour les raquettes, la balle et les scores
lv_obj_t *paddle1;
lv_obj_t *paddle2;
lv_obj_t *ball;
lv_obj_t *score_label1;
lv_obj_t *score_label2;

// Position des raquettes
int paddle1_y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2; // Raquette 1
int paddle2_y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2; // Raquette 2


int ball_x = SCREEN_WIDTH / 2;  //Position balle
int ball_y = SCREEN_HEIGHT / 2;  //Position balle
int ball_speed_x = BALL_SPEED_X;  //vitesse de la balle
int ball_speed_y = BALL_SPEED_Y;  //vitesse de la balle
// Scores des joueurs
int score1 = 0;
int score2 = 0;

// LED pour but
DigitalOut goal_led(LED_PIN);

// Fonction pour initialiser l'interface LVGL
void init_ui() {
    lv_obj_clear_flag (lv_scr_act (), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF); 


    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_bg_color(&style_btn, lv_color_black());
    lv_style_set_bg_opa(&style_btn, LV_OPA_50);
    lv_style_set_border_width(&style_btn, 2);
    lv_style_set_border_color(&style_btn, lv_color_black());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x7EBAB5), LV_PART_MAIN);

    // Créeattion la raquette 1
    paddle1 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(paddle1, PADDLE_WIDTH, PADDLE_HEIGHT);
    lv_obj_align(paddle1, LV_ALIGN_TOP_LEFT, 10, paddle1_y);

    // Créeation la raquette 2
    paddle2 = lv_obj_create(lv_scr_act());
    lv_obj_set_size(paddle2, PADDLE_WIDTH, PADDLE_HEIGHT);
    lv_obj_align(paddle2, LV_ALIGN_TOP_RIGHT, -10, paddle2_y);

    // Création la balle et la placer entre les raquettes ( au milieu)
    ball = lv_obj_create(lv_scr_act());
    lv_obj_set_size(ball, BALL_SIZE, BALL_SIZE);
    ball_x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
    ball_y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
    lv_obj_align(ball, LV_ALIGN_TOP_LEFT, ball_x, ball_y);


    static lv_style_t indicator_style;
    lv_style_init(&indicator_style); 


    // score pour les deux joueurs
   score_label1 = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(score_label1, "Joueur 1: %d", score1);
    lv_obj_align(score_label1, LV_ALIGN_TOP_LEFT, 10, 10);

    
    score_label2 = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(score_label2, "Joueur 2: %d", score2);
    lv_obj_align(score_label2, LV_ALIGN_TOP_RIGHT, -10, 10);
}

// Fonction mise à jour des score
void update_score_labels() {
    lv_label_set_text_fmt(score_label1, "Joueur 1: %d", score1);
    lv_label_set_text_fmt(score_label2, "Joueur 2: %d", score2);
}


void update_game_state() {
    // Lire l'état des boutons
    DigitalIn button_up(BUTTON_UP);
    DigitalIn button_down(BUTTON_DOWN);
    bool is_up_pressed = !button_up.read(); // boutons actifs bas
    bool is_down_pressed = !button_down.read(); // boutons actifs bas

    // Déplacer la raquette 2 (joueur à droite) avec les boutons
    if (is_up_pressed && paddle2_y > 0) {
        paddle2_y -= PADDLE_SPEED;
        lv_obj_set_y(paddle2, paddle2_y);
    }
    if (is_down_pressed && paddle2_y < (SCREEN_HEIGHT - PADDLE_HEIGHT)) {
        paddle2_y += PADDLE_SPEED;
        lv_obj_set_y(paddle2, paddle2_y);
    }

    // Déplacer la raquette 1 (joueur à gauche) avec le touchpad
    lv_indev_t *indev_touchpad = lv_indev_get_next(NULL);
    if(indev_touchpad != NULL && indev_touchpad->driver->type == LV_INDEV_TYPE_POINTER) {
    lv_point_t point;
    lv_indev_get_point(indev_touchpad, &point);
    paddle1_y = point.y;
    lv_obj_set_y(paddle1, paddle1_y);
}

    // Calculer les positions des bords des raquettes et de la balle
    int paddle1_left = 10; // Gauche de la raquette 1
    int paddle1_right = paddle1_left + PADDLE_WIDTH; // Droite de la raquette 1
    int paddle1_top = paddle1_y; // Haut de la raquette 1
    int paddle1_bottom = paddle1_top + PADDLE_HEIGHT; // Bas de la raquette 1
    
    int paddle2_right = SCREEN_WIDTH - 10; // Droite de la raquette 2
    int paddle2_left = paddle2_right - PADDLE_WIDTH; // Gauche de la raquette 2
    int paddle2_top = paddle2_y; // Haut de la raquette 2
    int paddle2_bottom = paddle2_top + PADDLE_HEIGHT; // Bas de la raquette 2
    
    int ball_left = ball_x;
    int ball_right = ball_left + BALL_SIZE;
    int ball_top = ball_y;
    int ball_bottom = ball_top + BALL_SIZE;

    // Déplacer la balle
    ball_x += ball_speed_x;
    ball_y += ball_speed_y;

    // Rebondir la balle sur les mur de l'écran
    if (ball_y <= 0 || ball_y >= (SCREEN_HEIGHT - BALL_SIZE)) {
        ball_speed_y = -ball_speed_y; // Inverser la direction verticale de la balle
    }

    // Rebondir la balle sur les raquettes
    if ((ball_x <= (paddle1_right + 5) && ball_x >= paddle1_left && 
         ball_y + BALL_SIZE >= paddle1_top && ball_y <= paddle1_bottom) ||
        (ball_x + BALL_SIZE >= (paddle2_left - 5) && ball_x + BALL_SIZE <= paddle2_right && 
         ball_y + BALL_SIZE >= paddle2_top && ball_y <= paddle2_bottom)) {
        ball_speed_x = -ball_speed_x; // Inverser la direction horizontale de la balle
    }

    // Empêcher la balle de sortir de l'écran
    if (ball_x <= 0) {
        ball_x = 0;
        ball_speed_x = -ball_speed_x;
        score2++; // Incrémenter le score du joueur 2
        update_score_labels(); // Mettre à jour les score
        goal_led = 1; // Allumer la LED pour indiquer un but
    } else if (ball_x >= (SCREEN_WIDTH - BALL_SIZE)) {
        ball_x = SCREEN_WIDTH - BALL_SIZE;
        ball_speed_x = -ball_speed_x;
        score1++; // Incrémenter le score du joueur 1
        update_score_labels(); // Mettre à jour les score
        goal_led = 1; // Allumer la LED pour indiquer un but
    } else {
        goal_led = 0; // Éteindre la LED si la balle est en jeu
    }

    // Mettre à jour la position de la balle
    lv_obj_set_pos(ball, ball_x, ball_y);
}

int main() {
    // Verrouiller le thread LVGL 
    threadLvgl.lock();

    // Initialiser l'interface  LVGL
    init_ui();

    // Déverrouiller le thread LVGL 
    threadLvgl.unlock();

    while (1) {
        // Verrouiller le thread LVGL pour mettre à jour l'affichage
        threadLvgl.lock();

        // Mettre à jour l'état du jeu
        update_game_state();

        // Déverrouiller le thread LVGL après avoir mis à jour l'affichage
        threadLvgl.unlock();

        // Dormir pendant 30ms avant de mettre à jour à nouveau
        ThisThread::sleep_for(30ms);
    }
}

