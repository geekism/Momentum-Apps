#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>

#define MOUSE_MOVE_SHORT 120

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    union {
        InputEvent input;
    };
    EventType type;
} UsbMouseEvent;

static void mouse_jiggler_render_callback(Canvas* canvas, void* ctx) {
    (void)ctx; // Mark ctx as unused
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Jiggle The Mouse");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 51, "Press Back to stop");
}

static void mouse_jiggler_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;

    UsbMouseEvent event;
    event.type = EventTypeInput;
    event.input = *input_event;
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

int32_t mouse_jiggler_app(void* p) {
    (void)p; // Mark p as unused

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(UsbMouseEvent));
    furi_check(event_queue);
    ViewPort* view_port = view_port_alloc();

    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_set_config(&usb_hid, NULL);

    view_port_draw_callback_set(view_port, mouse_jiggler_render_callback, NULL);
    view_port_input_callback_set(view_port, mouse_jiggler_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    UsbMouseEvent event;
    bool keep_running = true;

    while(keep_running) {
        // Move the mouse back and forth
        furi_hal_hid_mouse_move(MOUSE_MOVE_SHORT, 0);
        furi_delay_ms(500);
        furi_hal_hid_mouse_move(-MOUSE_MOVE_SHORT, 0);
        furi_delay_ms(500);

        // Check for any user input events
        while(furi_message_queue_get(event_queue, &event, 0) == FuriStatusOk) {
            if(event.type == EventTypeInput) {
                if(event.input.key == InputKeyBack) {
                    keep_running = false; // Stop jiggling if "Back" button is pressed
                }
            }
        }

        view_port_update(view_port);
    }

    // Restore previous USB configuration
    furi_hal_usb_set_config(usb_mode_prev, NULL);

    // Remove & free all resources created by the app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    return 0;
}
