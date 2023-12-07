#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>

#include <stdio.h>

#define HOT_COUNT_THRESHOLD 4

int count = 0; // Corretto il nome della variabile

static void vibration_hot_count() {
    notification_message = "Hot Count Threshold!";
    furi_hal_vibro_on(false);
    furi_hal_vibro_on(true);
    furi_delay_ms(1500);
    furi_hal_vibro_on(false);
    notificationString = "";
    return;
}

// static void app_draw_callback(Canvas* canvas, void* ctx) {
//     UNUSED(ctx);

//     canvas_clear(canvas);
//     char count_str[5];
//     snprintf(count_str, 5, "%d", count); // Converti il conteggio in stringa per disegnarlo
//     canvas_draw_str(canvas, 44, 24, count_str);
// }

static void app_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    char count_str[5];
    snprintf(count_str, 5, "%d", count);

    int text_width = canvas_glyph_width(canvas, count_str);
    int x = (128 - text_width) / 2;
    int y = 40; // Posizione y per centrare il testo in basso

    canvas_draw_str(canvas, x, y, count_str);

    // Aggiungi le informazioni sul conteggio delle carte
    canvas_draw_str(canvas, 0, 54, "Cards # 2-7= +1, 8 & 9= 0, and 10-Ace= -1");

    // Aggiungi la descrizione della vibrazione quando il conteggio è "hot"
    canvas_draw_str(canvas, 0, 60, "Vibrates the running count in pulses when count is hot");
}



static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}


int32_t blackjack_counter_main(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, view_port);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);
    
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    InputEvent event;

    bool running = true;
    while (running) {
        if (furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if ((event.type == InputTypePress) || (event.type == InputTypeRepeat)) {
                switch (event.key) {
                    case InputKeyUp:
                        count += 1;
                        break;
                    case InputKeyOk:
                        break;
                    case InputKeyDown:
                        count -= 1;
                        break;
                    default:
                        running = false;
                        break;
                }
                
                if (count >= HOT_COUNT_THRESHOLD || count <= -HOT_COUNT_THRESHOLD) {
                    vibration_hot_count();
                }
            }
            view_port_update(view_port);
        }
    }

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);

    return 0;
}
