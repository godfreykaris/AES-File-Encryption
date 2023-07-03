#pragma once

#include "aes.h"

HWND browse_button;
HICON browse_icon;
HWND browse_caption;

HWND filepath_field;
HWND file_caption;

HWND encryption_key_field;
HWND encryption_key_caption;
HWND encryption_key_pswdsz;
HWND encryption_key_on_off;
HWND show_password;

HFONT controls_font;

HWND encrypt_button;
HWND decrypt_button;

//window cooedinates
RECT window_rectangle;

RECT progress_windowrectangle;

RECT info_dialog_rectangle;

struct rectangle
{
	int bottom = 0;
	int top = 0;
	int left = 0;
	int right = 0;
};
rectangle previous_rectangle;
							//hght y    x   wdth
rectangle browse_rectangle = { 20, 260, 830, 26 };
rectangle browse_caption_rectangle = { 17, 280, 830, 40 };

rectangle filepath_field_rectangle = { 20, 260, 550, 280 };
rectangle file_caption_rectangle = { 20, 260, 490, 60 };

rectangle encryption_key_field_rectangle = { 20, 230, 550, 280 };
rectangle encryption_key_caption_rectangle = { 20, 230, 490, 60 };

rectangle encrypt_button_rectangle = { 20, 300, 560, 50 };
rectangle decrypt_button_rectangle = { 20, 300, 750, 50 };

int center1, center2;

int original_left = 26;
int original_top = 26;

int left_displacement;
int top_displacement;

//filepath variables
std::string filepath;
int filepath_length;

char pth[256] = "w";
LPSTR path = pth;

bool filepath_valid;

//password variables
std::string passwordstr;
int password_length;

char pswd[30] = "w";
LPSTR password = pswd;

const char* invalid_filepath = "invalid file path\0";

bool verify_filepath(const char* filepath)
{
	FILE* file = fopen(filepath, "rb");

	if (file == NULL)
		return false;
	else
		return true;
}

void encrypt(const char* filepath, const char* password)
{
	aes_encrypt_128_bits(filepath, (char*)password);
}

void decrypt(const char* filepath, const char* password)
{
	aes_decrypt_128_bits(filepath, (char*)password);
}

void setup_progressbar_data(const char* filepath)
{
	long filesize = (long)get_size(filepath);

	const int COMPLETE_BLOCK = 1024;
	int no_of_complete_blocks = filesize / COMPLETE_BLOCK;
	
	end = (int)((no_of_complete_blocks + 1) * 1.5);
}


void resize_callback(HWND hWnd)
{
    GetWindowRect(hWnd, &window_rectangle);

    center1 = ((window_rectangle.bottom - window_rectangle.top) / 2) + window_rectangle.top;
    center2 = ((window_rectangle.right - window_rectangle.left) / 2) + window_rectangle.left;

    left_displacement = window_rectangle.left - original_left;
    top_displacement = window_rectangle.top - original_top;
       
    if ((previous_rectangle.bottom != window_rectangle.bottom || previous_rectangle.top != window_rectangle.top ||
        previous_rectangle.left != window_rectangle.left || previous_rectangle.right != window_rectangle.right))//&& ((window_rectangle.right - window_rectangle.left) > 450))// && ((window_rectangle.right - window_rectangle.left) > 450))
    {
        if (window_rectangle.right != previous_rectangle.right)
        {
            browse_rectangle.left = center2 - (browse_rectangle.right / 2) + (filepath_field_rectangle.right / 2 + 15) - left_displacement;
            browse_caption_rectangle.left = browse_rectangle.left;

            filepath_field_rectangle.left = center2 - (filepath_field_rectangle.right / 2) - left_displacement;
            file_caption_rectangle.left = filepath_field_rectangle.left - file_caption_rectangle.right - 2;

            encryption_key_field_rectangle.left = center2 - (encryption_key_field_rectangle.right / 2) - left_displacement;
            encryption_key_caption_rectangle.left = encryption_key_field_rectangle.left - encryption_key_caption_rectangle.right - 2;

            encrypt_button_rectangle.left = file_caption_rectangle.left + 60;
            decrypt_button_rectangle.left = file_caption_rectangle.left + 270;

        }

        if (window_rectangle.bottom != previous_rectangle.bottom)
        {
            browse_rectangle.top = center1 - (browse_rectangle.bottom / 2) - 50 - top_displacement;
            browse_caption_rectangle.top = browse_rectangle.top + browse_rectangle.bottom;

            filepath_field_rectangle.top = center1 - (filepath_field_rectangle.bottom / 2) - 50 - top_displacement;
            file_caption_rectangle.top = filepath_field_rectangle.top;

            encryption_key_field_rectangle.top = filepath_field_rectangle.top - filepath_field_rectangle.bottom - 5;
            encryption_key_caption_rectangle.top = encryption_key_field_rectangle.top;

            encrypt_button_rectangle.top = file_caption_rectangle.top + 40;
            decrypt_button_rectangle.top = file_caption_rectangle.top + 40;

        }

        //save the window rectangle
        previous_rectangle.bottom = window_rectangle.bottom;
        previous_rectangle.top = window_rectangle.top;
        previous_rectangle.left = window_rectangle.left;
        previous_rectangle.right = window_rectangle.right;

        SetWindowPos(browse_button, NULL, browse_rectangle.left, browse_rectangle.top, browse_rectangle.right, browse_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(browse_caption, NULL, browse_caption_rectangle.left, browse_caption_rectangle.top, browse_caption_rectangle.right, browse_caption_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(filepath_field, NULL, filepath_field_rectangle.left, filepath_field_rectangle.top, filepath_field_rectangle.right, filepath_field_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(file_caption, NULL, file_caption_rectangle.left, file_caption_rectangle.top, file_caption_rectangle.right, file_caption_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(encryption_key_field, NULL, encryption_key_field_rectangle.left, encryption_key_field_rectangle.top, encryption_key_field_rectangle.right, encryption_key_field_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(encryption_key_caption, NULL, encryption_key_caption_rectangle.left, encryption_key_caption_rectangle.top, encryption_key_caption_rectangle.right, encryption_key_caption_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(encrypt_button, NULL, encrypt_button_rectangle.left, encrypt_button_rectangle.top, encrypt_button_rectangle.right, encrypt_button_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(decrypt_button, NULL, decrypt_button_rectangle.left, decrypt_button_rectangle.top, decrypt_button_rectangle.right, decrypt_button_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(encryption_key_pswdsz, NULL, browse_rectangle.left, encryption_key_field_rectangle.top, 18, browse_rectangle.bottom, SWP_SHOWWINDOW);
        SetWindowPos(encryption_key_on_off, NULL, encryption_key_field_rectangle.left + 30, encryption_key_field_rectangle.top - 20, 15, browse_rectangle.bottom - 5, SWP_SHOWWINDOW);
        SetWindowPos(show_password, NULL, encryption_key_field_rectangle.left + 50, encryption_key_field_rectangle.top - 20, 90, browse_rectangle.bottom - 5, SWP_SHOWWINDOW);


    }
}