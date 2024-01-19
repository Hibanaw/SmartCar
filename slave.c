#include "slave.h"

#include <math.h>
#include "zf_common_headfile.h"
#include "param.h"
#include "flash.h"
#include "button.h"
#include "switch.h"
#include "image.h"
#include "encoder.h"
#include "control.h"

int8 page = 0;
int8 option = 0;
int8 focusLevel = 0;

int8 editPos = 0;

int8 showImageLine = 1;

uint8 pageCount = 4;

const char *routeStateText[] = {
    [PARK_IN] = "PrkIn",
    [PARK_OUT] = "PrkOut",
    [STOP] = "Stop!!",
    [STRAIGHT] = "Strght",
    [LEFT_ROUNDABOUT_IN_0] = "LRdIn0",
    [LEFT_ROUNDABOUT_IN_1] = "LRdIn1",
    [LEFT_ROUNDABOUT_KEEP] = "LRKeep",
    [LEFT_ROUNDABOUT_OUT] = "LRdOut",
    [RIGHT_ROUNDABOUT_IN_0] = "RRdIn0",
    [RIGHT_ROUNDABOUT_IN_1] = "RRdIn1",
    [RIGHT_ROUNDABOUT_KEEP] = "RRKeep",
    [RIGHT_ROUNDABOUT_OUT] = "RRdOut",
    [CROSS] = "Crosss",
    [UPHILL] = "Uphill",
};

struct SlavePage slavePage[5] = {
    {.title = "Moni."},
    {
        .title = "Basic",
        .option = {
            {.name = "AtClk", .type = SWITCH},
            {.name = "DoPrk", .type = SWITCH, .data = 1},
            {.name = "Rst", .type = SWITCH, .data = 1},
            {.name = "Spd", .type = VALUE, .lRange = 0, .rRange = 1000, .data = 290},
            {.name = "EdgF", .type = SWITCH, .data = 1},
        },
        .optionCount = 5,
    },
    {
        .title = "Cont.",
        .option = {
            {.name = "ProU", .type = VALUE, .lRange = 0, .rRange = 127, .data = 10},
            {.name = "ProL", .type = VALUE, .lRange = 0, .rRange = 127, .data = 50},
            {.name = "StP", .type = VALUE, .data = 2400},
            {.name = "StD", .type = VALUE, .data = 650},
            {.name = "MoP", .type = VALUE, .data = 1000},
            {.name = "MoI", .type = VALUE, .data = 50},
            {.name = "MoD", .type = VALUE, .data = 40},
        },
        .optionCount = 7,
    },
    {.title = "About"}};

void slaveSave()
{
    for (int i = 0; i < pageCount; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            flash_union_buffer[i * 15 + j].int32_type = slavePage[i].option[j].data;
        }
    }
    saveParameterToFlash();
    isAntiClockWise = slavePage[1].option[0].data;
    doOutPark = slavePage[1].option[1].data;
    reset = slavePage[1].option[2].data;
    speed = slavePage[1].option[3].data;
    doEdgeFix = slavePage[1].option[4].data;
    prospectU = slavePage[2].option[0].data;
    prospectL = slavePage[2].option[1].data;
    steerKp = slavePage[2].option[2].data;
    steerKd = slavePage[2].option[3].data;
    motorKp = slavePage[2].option[4].data;
    motorKi = slavePage[2].option[5].data;
    motorKd = slavePage[2].option[6].data;
}

void slaveInit()
{
    readParameterFromFlash();
    for (int i = 0; i < pageCount; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            if (!slavePage[i].option[j].data)
                slavePage[i].option[j].data = flash_union_buffer[i * 15 + j].int32_type;
            if (!slavePage[i].option[j].lRange && !slavePage[i].option[j].rRange)
            {
                slavePage[i].option[j].rRange = 99999;
                slavePage[i].option[j].lRange = 0;
            }
        }
    }
    slaveSave();

    tft180_set_font(TFT180_6X8_FONT);
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
}

void slaveAbout()
{
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_set_font(TFT180_6X8_FONT);
    tft180_show_string(50, 30, "HUST");
    tft180_show_string(35, 40, "10.Hu Tianqi");
    tft180_show_rgb565_image(32, 70, image_data, 64, 63, 64, 64, 0);
}

void slaveWelcome()
{
    tft180_clear();
    slaveAbout();
    system_delay_ms(1000);
}

void slaveEvent()
{
    buttonScan();
    // switchScan();
    if (slavePage[page].title == "Moni.")
        slaveShowStatus();
    slaveShowImage();
    // 若拨片拨开，按位修??
    // if (switch1 && focusLevel == 2)
    // {
    //     focusLevel = 3;
    //     slaveDraw();
    // }
    // if (!switch1 && focusLevel == 3)
    // {
    //     focusLevel--;
    //     slaveDraw();
    // }
    // if (switch2 && focusLevel != 0)
    // {
    //     focusLevel = 0;
    //     option = 0;
    //     slaveDraw();
    // }
    // if ((slavePage[page].title == "About" || slavePage[page].title == "Moni.") && focusLevel != 0)
    // {
    //     focusLevel = 0;
    // }
    if (!g_button)
        return;
    switch (focusLevel)
    {
    case 0:
        switch (g_button)
        {
        case SLAVEBUTTON_DOWN:
            page++;
            break;
        case SLAVEBUTTON_UP:
            page--;
            break;
        case SLAVEBUTTON_RIGHT:
            focusLevel++;
            break;
        default:
            break;
        }
        page = page < 0 ? pageCount - 1 : page;
        page = page >= pageCount ? 0 : page;
        break;
    case 1:
        switch (g_button)
        {
        case SLAVEBUTTON_DOWN:
            option++;
            break;
        case SLAVEBUTTON_UP:
            option--;
            break;
        case SLAVEBUTTON_RIGHT:
            focusLevel++;
            slavePage[page].option[option].tempData = slavePage[page].option[option].data;
            break;
        case SLAVEBUTTON_LEFT:
            focusLevel--;
            break;
        default:
            break;
        }
        option = option < 0 ? slavePage[page].optionCount - 1 : option;
        option = option >= slavePage[page].optionCount ? 0 : option;
        break;
    case 2:
        switch (g_button)
        {
        case SLAVEBUTTON_UP:
            if (slavePage[page].option[option].type == VALUE)
            {
                slavePage[page].option[option].tempData += pow(10, editPos);
                if (slavePage[page].option[option].lRange || slavePage[page].option[option].rRange)
                {
                    slavePage[page].option[option].tempData =
                        slavePage[page].option[option].tempData < slavePage[page].option[option].lRange
                            ? slavePage[page].option[option].lRange
                            : slavePage[page].option[option].tempData;
                    slavePage[page].option[option].tempData =
                        slavePage[page].option[option].tempData > slavePage[page].option[option].rRange
                            ? slavePage[page].option[option].rRange
                            : slavePage[page].option[option].tempData;
                }
            }
            else
                slavePage[page].option[option].tempData = !slavePage[page].option[option].tempData;
            break;
        case SLAVEBUTTON_DOWN:
            if (slavePage[page].option[option].type == VALUE)
            {
                slavePage[page].option[option].tempData -= pow(10, editPos);
                if (slavePage[page].option[option].lRange || slavePage[page].option[option].rRange)
                {
                    slavePage[page].option[option].tempData =
                        slavePage[page].option[option].tempData < slavePage[page].option[option].lRange
                            ? slavePage[page].option[option].rRange
                            : slavePage[page].option[option].tempData;
                    slavePage[page].option[option].tempData =
                        slavePage[page].option[option].tempData > slavePage[page].option[option].rRange
                            ? slavePage[page].option[option].lRange
                            : slavePage[page].option[option].tempData;
                }
            }
            else
                slavePage[page].option[option].tempData = !slavePage[page].option[option].tempData;
            break;
        case SLAVEBUTTON_RIGHT:
            focusLevel++;
            break;
        case SLAVEBUTTON_LEFT:
            slavePage[page].option[option].data = slavePage[page].option[option].tempData;
            focusLevel--;
            slaveSave();
            break;
        default:
            break;
        }
        break;
    case 3:
        switch (g_button)
        {
        case SLAVEBUTTON_UP:
            editPos++;
            break;
        case SLAVEBUTTON_DOWN:
            editPos--;
            break;
        case SLAVEBUTTON_RIGHT:
        case SLAVEBUTTON_LEFT:
            focusLevel--;
            break;
        default:
            break;
        }
        editPos = editPos < 0 ? 4 : editPos;
        editPos = editPos > 4 ? 0 : editPos;
        break;
    default:
        break;
    }
    slaveDraw();
}

void slaveDraw()
{
    tft180_full(RGB565_WHITE);
    tft180_set_font(TFT180_6X8_FONT);
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_draw_line(0, 148, 127, 148, RGB565_GRAY);
    for (int i = 0; i < pageCount; i++)
    {
        if (i == page)
            tft180_set_color(RGB565_BLACK, RGB565_WHITE);
        else
            tft180_set_color(RGB565_GRAY, RGB565_WHITE);
        tft180_show_string(4 + i * 30, 151, slavePage[i].title);
    }
    for (int i = 0; i < slavePage[page].optionCount; i++)
    {
        int ty = 15 + i * 10;
        if (focusLevel == 1 && i == option)
            tft180_set_color(RGB565_BLACK, RGB565_WHITE);
        else
            tft180_set_color(RGB565_GRAY, RGB565_WHITE);
        tft180_show_string(5, ty, slavePage[page].option[i].name);
        switch (slavePage[page].option[i].type)
        {
        case SWITCH:
            tft180_show_string(50, ty, "Switch");
            tft180_show_int(90, ty, slavePage[page].option[i].data, 1);
            break;
        default:
            tft180_show_string(50, ty, "Value");
            tft180_show_int(90, ty, slavePage[page].option[i].data, 5);
            break;
        }
        if (focusLevel >= 2 && i == option)
        {
            tft180_set_color(RGB565_BLACK, RGB565_RED);
            tft180_show_int(90, ty, slavePage[page].option[i].tempData, 5);
        }
    }
    if (focusLevel == 3)
        tft180_set_color(RGB565_WHITE, RGB565_RED);
    else if (focusLevel == 2)
        tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    else
        tft180_set_color(RGB565_GRAY, RGB565_WHITE);
    tft180_show_uint(115, 5, editPos + 1, 1);

    if (slavePage[page].title == "About")
        slaveAbout();
    if (slavePage[page].title == "Moni.")
        slaveShowImage();
}

void slaveShowStatus()
{
    tft180_draw_line(0, 64, 127, 64, RGB565_GRAY);
    tft180_set_color(RGB565_BLACK, RGB565_WHITE);
    tft180_show_int(0, 70, lMotorOutput, 4);
    tft180_show_int(40, 70, rMotorOutput, 4);
    tft180_show_int(0, 80, threshold, 3);
    tft180_show_int(0, 90, steerError, 3);
    tft180_show_string(0, 100, routeStateText[routeState]);
    tft180_show_float(0, 110, lSpeed, 3, 1);
    tft180_show_float(30, 110, rSpeed, 3, 1);
    tft180_show_float(0, 120, speedTarget, 3, 1);
    tft180_show_int(0, 130, fPoints.Cx, 2);
    tft180_show_int(20, 130, fPoints.Cy, 2);
    tft180_show_int(40, 130, fPoints.Dx, 2);
    tft180_show_int(60, 130, fPoints.Dy, 2);
    tft180_show_int(0, 140, fPoints.Ex, 2);
    tft180_show_int(20, 140, fPoints.Ey, 2);
    tft180_show_int(40, 140, fPoints.Fx, 2);
    tft180_show_int(60, 140, fPoints.Fy, 2);
    // tft180_draw_point(fPoints.Cx, fPoints.Cy, RGB565_GREEN);
    // tft180_draw_point(fPoints.Dx, fPoints.Dy, RGB565_GREEN);
    // tft180_draw_point(fPoints.Ex, fPoints.Ey, RGB565_GREEN);
    // tft180_draw_point(fPoints.Fx, fPoints.Fy, RGB565_GREEN);

    // if (showImageLine)
    // {
    //     // tft180_draw_line(maxlpx, 63 - maxl, maxlpx, 63,
    //     //                  RGB565_YELLOW); // 最长白线
    //     // tft180_draw_line(IMAGEMIDLINE, 0, IMAGEMIDLINE, 63,
    //     //                  RGB565_BLUE); // 图像中线
    //     for (int i = 63 - maxl; i < 64; i++)
    //     {
    //         if (lEdge[i] >= 0 && lEdge[i] < 128)
    //             tft180_draw_point(lEdge[i], i, RGB565_BLUE); // 左边线
    //         if (rEdge[i] >= 0 && rEdge[i] < 128)
    //             tft180_draw_point(rEdge[i], i, RGB565_BLUE); // 右边线
    //         if (roadMid[i] >= 0 && roadMid[i] < 128)
    //             tft180_draw_point(roadMid[i], i, RGB565_RED); // 道路中线
    //     }
    //     tft180_draw_line(0, prospectL, 127, prospectL,
    //                      RGB565_CYAN); // 前瞻上边线
    //     tft180_draw_line(0, prospectU, 127, prospectU,
    //                      RGB565_PURPLE); // 前瞻下边线
    // }
}

void slaveShowImage()
{
    if (slavePage[page].title != "Moni.")
        return;
    if (1)
        tft180_show_gray_image(0, 0, *image, 128, 64, 128, 64, 0);
    else
    {
        tft180_clear();
    }
}
