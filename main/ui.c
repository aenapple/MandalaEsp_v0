#include "ui.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>


static lv_obj_t *img_obj;
static char image_files[32][128];
static int image_count = 0;
static int current = 0;

static void fade_exec(void *var, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)var, v, LV_PART_MAIN);
}

static void fade_out_done(lv_anim_t *a)
{
    current = (current + 1) % image_count;
    lv_img_set_src(img_obj, image_files[current]);

    lv_anim_t ai;
    lv_anim_init(&ai);
    lv_anim_set_var(&ai, img_obj);
    lv_anim_set_values(&ai, 0, 255);
    lv_anim_set_time(&ai, 500);
    lv_anim_set_exec_cb(&ai, fade_exec);
    lv_anim_start(&ai);
}

static void switch_image(lv_timer_t *t)
{
    lv_anim_t ao;
    lv_anim_init(&ao);
    lv_anim_set_var(&ao, img_obj);
    lv_anim_set_values(&ao, 255, 0);
    lv_anim_set_time(&ao, 500);
    lv_anim_set_exec_cb(&ao, fade_exec);
    lv_anim_set_ready_cb(&ao, fade_out_done);
    lv_anim_start(&ao);
}

static void scan_folder(const char *path)
{
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL)
    {
        if (strstr(ent->d_name, ".png") || strstr(ent->d_name, ".jpg"))
        {
//   aen         snprintf(image_files[image_count], sizeof(image_files[image_count]),
//   aen                  "%s/%s", path, ent->d_name);
            image_count++;
            if (image_count >= 32) break;
        }
    }
    closedir(dir);
}

void ui_init(void)
{
    scan_folder("/sdcard/images");

    img_obj = lv_img_create(lv_scr_act());
    lv_img_set_src(img_obj, image_files[0]);
    lv_obj_center(img_obj);
    lv_obj_set_style_opa(img_obj, 255, LV_PART_MAIN);

    lv_timer_create(switch_image, 3000, NULL);
}
