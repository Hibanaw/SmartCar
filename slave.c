#include "slave.h"

int8 page = 0;
int8 option = 0;
int8 focusLevel = 0;

int8 editPos = 0;

int8 showImageLine = 1;

const uint8 pageCount = 4;
struct SlavePage slavePage[5] = {
    {"Home",
     {{"DoBin", SWITCH},
      {"DoOtsu", SWITCH},
      {"SetT", VALUE},
      {"DoCal", SWITCH}},
     4},
    {"Cont.",
     {{"ProU", VALUE, 0, 127},
      {"ProL", VALUE, 0, 127}},
     2},
    {"Moni."},
    {"About"}};

void slaveInit()
{
    readParameterFromFlash();
    for (int i = 0; i < pageCount; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            slavePage[i].option[j].data = flash_union_buffer[i * 5 + j].int32_type;
            if (!slavePage[i].option[j].lRange && !slavePage[i].option[j].rRange)
            {
                slavePage[i].option[j].rRange = 99999;
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
    tft180_show_string(35, 40, "11.Hu Tianqi");
    tft180_show_rgb565_image(32, 70, image_data, 64, 63, 64, 64, 0);
}

void slaveWelcome()
{
    tft180_clear();
    slaveAbout();
    system_delay_ms(1000);
}

void slaveSave()
{
    for (int i = 0; i < pageCount; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            flash_union_buffer[i * 5 + j].int32_type = slavePage[i].option[j].data;
        }
    }
    saveParameterToFlash();
    doBin = slavePage[0].option[0].data;
    calThres = slavePage[0].option[1].data;
    threshold = slavePage[0].option[2].data;
    doCal = slavePage[0].option[3].data;
}

void slaveEvent()
{
    buttonScan();
    switchScan();
    if (slavePage[page].title == "Moni.")
        slaveShowStatus();
    // 若拨片拨开，按位修改
    if (switch1 && focusLevel == 2)
    {
        focusLevel = 3;
        slaveDraw();
    }
    if (!switch1 && focusLevel == 3)
    {
        focusLevel--;
        slaveDraw();
    }
    if (switch2 && focusLevel != 0)
    {
        focusLevel = 0;
        option = 0;
        slaveDraw();
    }
    if ((slavePage[page].title == "About" || slavePage[page].title == "Moni.") && focusLevel != 0)
    {
        focusLevel = 0;
    }
    if (!g_button)
        return;
    switch (focusLevel)
    {
    case 0:
        switch (g_button)
        {
        case SLAVEBUTTONDOWN:
            page++;
            break;
        case SLAVEBUTTONUP:
            page--;
            break;
        case SLAVEBUTTONOK:
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
        case SLAVEBUTTONDOWN:
            option++;
            break;
        case SLAVEBUTTONUP:
            option--;
            break;
        case SLAVEBUTTONOK:
            focusLevel++;
            slavePage[page].option[option].tempData = slavePage[page].option[option].data;
            break;
        case SLAVEBUTTONBACK:
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
        case SLAVEBUTTONUP:
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
        case SLAVEBUTTONDOWN:
            if (slavePage[page].option[option].type == VALUE)
            {
                slavePage[page].option[option].tempData -= pow(10, editPos);
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
        case SLAVEBUTTONOK:
            slavePage[page].option[option].data = slavePage[page].option[option].tempData;
            focusLevel--;
            slaveSave();
            break;
        case SLAVEBUTTONBACK:
            focusLevel--;
            break;
        default:
            break;
        }
        break;
    case 3:
        switch (g_button)
        {
        case SLAVEBUTTONUP:
            editPos++;
            break;
        case SLAVEBUTTONDOWN:
            editPos--;
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
    tft180_show_int(0, 80, threshold, 3);
    tft180_show_int(0, 90, error, 5);
    if (showImageLine)
    {
        tft180_draw_line(maxlpx, 63 - maxl, maxlpx, 63,
                         RGB565_YELLOW); // 最长白列
        tft180_draw_line(IMAGEMIDLINE, 0, IMAGEMIDLINE, 63,
                         RGB565_BLUE); // 图像中线
        for (int i = 63 - maxl; i < 64; i++)
        {
            tft180_draw_point(lEdge[i], i, RGB565_RED);   // 左边缘
            tft180_draw_point(rEdge[i], i, RGB565_RED);   // 右边缘
            tft180_draw_point(roadMid[i], i, RGB565_RED); // 道路中线
        }
        tft180_draw_line(0, prospectL, 127, prospectL,
                         RGB565_CYAN); // 前瞻上边沿
        tft180_draw_line(0, prospectU, 127, prospectU,
                         RGB565_PURPLE); // 前瞻下边沿
    }
}

void slaveShowImage()
{
    if (slavePage[page].title != "Moni.")
        return;
    tft180_show_gray_image(0, 0, *image, 128, 64, 128, 64, 0);
}
