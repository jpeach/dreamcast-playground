    #include <kos.h>

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include <cstdio>

KOS_INIT_FLAGS(INIT_DEFAULT);

template <typename T, unsigned N>
unsigned array_size(const T (&array)[N])
{
    return N;
}

template <typename T>
void zero_memory(T * ptr, unsigned count)
{
    ::memset(ptr, 0, sizeof(T) * count);
}

// Given a vram pointer of pixel dimensions Width * Height, return a
// pointer to the position given by x and y in the vram buffer.
template<unsigned Width, unsigned Height, typename T>
T *linearize_point(T * vram, unsigned x, unsigned y)
{
    x = std::min(x, Width);
    y = std::min(y, Height);

    return vram + x + (Width * y);
}

void scan_maple_device(const std::string& name, uint32_t func)
{
    maple_device_t * dev;

    for (int i = 0; ; ++i) {
        dev = maple_enum_type(i, func);
        if (!dev) {
            break;
        }

        std::fprintf(stderr, "\tvalid: %d\n", dev->valid);

        // Experimentally, for controllers, port and unit IDs change
        // together. Port 0, unit 0 is the first controller, and port 3,
        // unit 3 is the 4th controller.
        std::fprintf(stderr, "\tport: %d\n", dev->port);
        std::fprintf(stderr, "\tunit: %d\n", dev->unit);

        std::fprintf(stderr, "\tinfo.functions: %s\n", maple_pcaps(dev->info.functions));
        std::fprintf(stderr, "\tinfo.area_code: %d\n", dev->info.area_code);
        std::fprintf(stderr, "\tinfo.connector_direction: %d\n", dev->info.connector_direction);
        std::fprintf(stderr, "\tinfo.product_name: %s\n", dev->info.product_name);
        std::fprintf(stderr, "\tinfo.product_license: %s\n", dev->info.product_license);
        std::fprintf(stderr, "\tinfo.standby_power: %d\n", dev->info.standby_power);
        std::fprintf(stderr, "\tinfo.max_power: %d\n", dev->info.max_power);
    }
}

std::string fmt_button_state(uint32 buttons)
{
    const struct {const char * name; uint32 value; } button_names[] = {
#define NAMED_BUTTON(name) { #name, name }
        NAMED_BUTTON(CONT_A),
        NAMED_BUTTON(CONT_B),
        NAMED_BUTTON(CONT_C),
        NAMED_BUTTON(CONT_D),
        NAMED_BUTTON(CONT_X),
        NAMED_BUTTON(CONT_Y),
        NAMED_BUTTON(CONT_Z),
        NAMED_BUTTON(CONT_START),
        NAMED_BUTTON(CONT_DPAD_UP),
        NAMED_BUTTON(CONT_DPAD_DOWN),
        NAMED_BUTTON(CONT_DPAD_LEFT),
        NAMED_BUTTON(CONT_DPAD_RIGHT),
        NAMED_BUTTON(CONT_DPAD2_UP),
        NAMED_BUTTON(CONT_DPAD2_DOWN),
        NAMED_BUTTON(CONT_DPAD2_LEFT),
        NAMED_BUTTON(CONT_DPAD2_RIGHT),
#undef NAMED_BUTTON
    };

    std::string result;

    result.reserve(array_size(button_names) * 10);

    for (unsigned i = 0; i < array_size(button_names); ++i) {
        if ((buttons & button_names[i].value) == 0) {
            continue;
        }

        if (!result.empty()) {
            result.append(" | ");
        }

        result.append(button_names[i].name);
    }

    return result;
}

void scan_controllers()
{
    for (int i = 0; ; ++i) {
        maple_device_t * dev = maple_enum_type(i, MAPLE_FUNC_CONTROLLER);
        if (!dev) {
            break;
        }

        cont_state_t * state = static_cast<cont_state_t *>(maple_dev_status(dev));
        if (!state) {
            break;
        }

        // We want to write each controller state to a separate line, where the
        // line number is stable, and the first 4 lines are skipped.
        auto lineno = 4 + dev->port;
        auto pos = linearize_point<640, 480>(vram_s, 0, BFONT_HEIGHT * lineno);

        // Clear the full width of the vram for the string we are about to draw.
        zero_memory(pos, 640 * BFONT_HEIGHT);

        // Approximate the numbber of characters we can draw in a single line.
        char line[640/BFONT_THIN_WIDTH];

        // Draw the current buttons state.
        snprintf(line, sizeof(line),
            "(%d,%d): %s", dev->unit, dev->port,fmt_button_state(state->buttons).c_str());

        bfont_draw_str(pos, 640, 1, line);
    }
}

void setup_exit_sequence()
{
    const uint32 exit_sequence = CONT_START | CONT_A | CONT_B | CONT_X | CONT_Y;
    auto exit_callback = [](uint8 addr, uint32 btns) {
        arch_exit();
    };

    std::fprintf(stderr, "setting up exit sequence\n");

    // Register the exit func on all controllers (i.e. address zero).
    cont_btn_callback(0, exit_sequence, exit_callback);

    auto start = linearize_point<640, 480>(vram_s, 0, BFONT_HEIGHT);
    bfont_draw_str(start, 640, 0, "To exit press");

    bfont_set_encoding(BFONT_CODE_RAW);

    start += (BFONT_THIN_WIDTH * 14);

    bfont_draw_wide(start, 640, 1, BFONT_ABUTTON);
    start += (BFONT_WIDE_WIDTH);

    bfont_draw_wide(start, 640, 1, BFONT_BBUTTON);
    start += (BFONT_WIDE_WIDTH);

    bfont_draw_wide(start, 640, 1, BFONT_XBUTTON);
    start += (BFONT_WIDE_WIDTH);

    bfont_draw_wide(start, 640, 1, BFONT_YBUTTON);
    start += (BFONT_WIDE_WIDTH);

    bfont_draw_wide(start, 640, 1, BFONT_STARTBUTTON);
    start += (BFONT_WIDE_WIDTH);

    bfont_set_encoding(BFONT_CODE_ISO8859_1);
}

int main(int argc [[maybe_unused]], const char ** argv [[maybe_unused]])
{
    std::fprintf(stderr, "Controller Playground\n");

    vid_init(DEFAULT_VID_MODE, DEFAULT_PIXEL_MODE);
    vid_set_mode(DM_640x480, PM_RGB565);
    bfont_set_encoding(BFONT_CODE_ISO8859_1);

    setup_exit_sequence();

    int ndev = maple_enum_count();
    std::fprintf(stderr, "total device count: %d\n", ndev);

    std::map<uint32_t, std::string> device_types {
        { MAPLE_FUNC_PURUPURU, "purupuru" },
        { MAPLE_FUNC_MOUSE, "mouse" },
        { MAPLE_FUNC_CAMERA, "camera" },
        { MAPLE_FUNC_CONTROLLER, "controller" },
        { MAPLE_FUNC_MEMCARD, "memcard" },
        { MAPLE_FUNC_LCD, "lcd" },
        { MAPLE_FUNC_CLOCK, "clock" },
        { MAPLE_FUNC_MICROPHONE, "microphone" },
        { MAPLE_FUNC_ARGUN, "argun" },
        { MAPLE_FUNC_KEYBOARD, "keyboard" },
        { MAPLE_FUNC_LIGHTGUN, "lightgun" },
    };

    for (auto&& [func, name] : device_types) {
        std::fprintf(stderr, "%s:\n", name.c_str());
        scan_maple_device(name, func);
    }

    for (;;) {
        thd_sleep(std::chrono::milliseconds(100).count());
        scan_controllers();
    }

    return 0;
}
