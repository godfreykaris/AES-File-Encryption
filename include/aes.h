#include <iostream>
#include <string>

//disable deprecation warning
#pragma warning(disable: 4996)

using byte = unsigned char;
using word = byte[4];
using key = word[4];

void substitute_bytes(byte state[4][4]);
void Inv_substitute_bytes(byte state[4][4]);

void shiftrows(byte state[4][4]);
void Invshiftrows(byte state[4][4]);

byte GFMul(byte Galois_field_byte, byte State_byte);
void MixColumns(byte state[4][4]);
void InvMixColumns(byte state[4][4]);

void xor_word(word w1, word w2);
void rotate_word(word w);
void substitute_word(word w);

void KeyExpansion(byte key[4][4], word expanded_keys[44]);
void store_keys(key keys[11], word expandedkeys[44]);
void AddRoundKey(byte state[4][4], byte round_key[4][4]);

void encrypt(byte state[4][4], byte round_key[4][4], int round);
void dencrypt(byte state[4][4], byte round_key[4][4],int round);

void cipher(char* data, key keys[11], int bunches_of_state, char* out);
void inv_cipher(char* data, key keys[11], int bunches_of_state, char* out);

void aes_encrypt_128_bits(const char* filepath, char* encryption_key);
void aes_decrypt_128_bits(const char* filepath, char* decryption_key);

std::string get_directory(const char* filepath);
int64_t get_size(const char* filepath);

void copy(byte arr1[4][4], byte arr2[4][4]);
void copy(byte arr[4][4], char* data, int index, std::string where);

void align(byte arr[4][4], byte unaligned_arr[4][4]);

//default state
char* defaultstate = (char*)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
byte default_state[4][4];

HWND hWndprogress;
HWND hwndprogress;

int complete_task = 0;
bool storing = false;

int start = 0;
int end = 0;

int pb_pos = 0;

std::string get_directory(const char* filepath)
{
    std::string path = filepath;
    int last_of_slash = path.find_last_of("\\");

    if (std::string::npos != last_of_slash) //if the path is not empty
        path = path.substr(0, last_of_slash);

    return path;
}

void aes_encrypt_128_bits(const char* filepath, char* encryption_key)
{
    long filesize = (long)get_size(filepath);

    std::string template_filepath = get_directory(filepath);
    template_filepath.append("\\temp.txt");

    const int COMPLETE_BLOCK = 1024;
    int no_of_complete_blocks = filesize / COMPLETE_BLOCK;
    int sizeof_incomplete = filesize - (no_of_complete_blocks * COMPLETE_BLOCK);

    char* data = new char[COMPLETE_BLOCK];
    memset(data, 0, COMPLETE_BLOCK + 1);

    char* out = new char[COMPLETE_BLOCK];
    memset(out, 0, COMPLETE_BLOCK + 1);

    byte mykey[4][4];
    copy(mykey, encryption_key, 0, "state");

    int rounds = 11;
    key keys[11];
    word expanded_keys[44];

    KeyExpansion(mykey, expanded_keys);
    store_keys(keys, expanded_keys);

    copy(default_state, defaultstate, 0, "state");

    FILE* f = fopen(filepath, "rb"), * f1 = fopen(template_filepath.c_str(), "wb");

    int remainder = filesize % 16;

    std::string incomplete_data_block;

    ShowWindow(hWndprogress, SW_SHOW);
    
    for (int i = 0; i < no_of_complete_blocks; i++)
    {
        fread(data, sizeof(char), COMPLETE_BLOCK, f);
        //encrypt
        cipher(data, keys, (COMPLETE_BLOCK / 16), out);
        //store in a temporary file
        fwrite(out, sizeof(char), COMPLETE_BLOCK, f1);

        memset(out, 0, COMPLETE_BLOCK + 1);
        memset(data, 0, COMPLETE_BLOCK + 1);
        
        SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
        complete_task++;
       
    }
    if (sizeof_incomplete > 0)
    {
        SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
        complete_task++;

        fread(data, sizeof(char), sizeof_incomplete, f);
       
        if ((sizeof_incomplete % 16) != 0)
        {
            //encrypt
            cipher(data, keys, (sizeof_incomplete / 16) + 1, out);
            //store in a temporary file
            fwrite(out, sizeof(char), sizeof_incomplete + (16 - remainder), f1);
        }         
        else
        {
            //encrypt
            cipher(data, keys, (sizeof_incomplete / 16), out);
            //store in a temporary file
            incomplete_data_block.append(out);
            fwrite(out, sizeof(char), incomplete_data_block.size(), f1);
        }  


    }

    fclose(f);
    fclose(f1);

    f1 = fopen(template_filepath.c_str(), "rb");
    f = fopen(filepath, "wb");

    sizeof_incomplete = (int)get_size(template_filepath.c_str());

    storing = true;
    SendMessage(hwndprogress, PBM_SETSTEP, (WPARAM)2, 0);
    //store encrypted data
    for (int i = 0; i < no_of_complete_blocks; i++)
    {        
        if (complete_task % 2 == 0)
        {
            SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
            complete_task++;
        }
        fread(data, sizeof(char), COMPLETE_BLOCK, f1);
        fwrite(data, sizeof(char), COMPLETE_BLOCK, f);

        complete_task++;
    }
    if (sizeof_incomplete > 0)
    {
       if (complete_task % 2 == 0)
        {
           SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
           complete_task++;
        }
        fread(data, sizeof(char), sizeof_incomplete, f1);
        fwrite(data, sizeof(char), sizeof_incomplete, f);

        complete_task++;
    }

    SendMessage(hWndprogress, WM_CLOSE, NULL, 0);
    complete_task = 0;

    fclose(f);
    fclose(f1);

    remove(template_filepath.c_str());
}

void aes_decrypt_128_bits(const char* filepath, char* decryption_key)
{
    long filesize = (long)get_size(filepath);

    std::string template_filepath = get_directory(filepath);
    template_filepath.append("\\temp.txt");

    const int COMPLETE_BLOCK = 1024;
    int no_of_complete_blocks = filesize / COMPLETE_BLOCK;
    int sizeof_incomplete = filesize - (no_of_complete_blocks * COMPLETE_BLOCK);

    char* data = new char[COMPLETE_BLOCK];
    memset(data, 0, COMPLETE_BLOCK + 1);

    char* out = new char[COMPLETE_BLOCK];
    memset(out, 0, COMPLETE_BLOCK + 1);

    byte mykey[4][4];
    copy(mykey, decryption_key, 0, "state");

    int rounds = 11;
    key keys[11];
    word expanded_keys[44];

    KeyExpansion(mykey, expanded_keys);
    store_keys(keys, expanded_keys);

    copy(default_state, defaultstate, 0, "state");

    FILE *f = fopen(filepath, "rb"), *f1 = fopen(template_filepath.c_str(), "wb");

    memset(out, 0, COMPLETE_BLOCK + 1);
    memset(data, 0, COMPLETE_BLOCK + 1);

    int remainder = filesize % 16;

    std::string incomplete_data_block;

    ShowWindow(hWndprogress, SW_SHOW);

    for (int i = 0; i < no_of_complete_blocks; i++)
    {
        fread(data, sizeof(char), COMPLETE_BLOCK, f);
        //decrypt
        inv_cipher(data, keys, (COMPLETE_BLOCK / 16), out);
        //store in a temporary file
        fwrite(out, sizeof(char), COMPLETE_BLOCK, f1);

        memset(out, 0, COMPLETE_BLOCK + 1);
        memset(data, 0, COMPLETE_BLOCK + 1);

        SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
        complete_task++;
    }
    if (sizeof_incomplete > 0)
    {
        SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
        complete_task++;

        if ((sizeof_incomplete % 16) != 0)
        {
            fread(data, sizeof(char), sizeof_incomplete + (16 - remainder), f);
            //decrypt 
            inv_cipher(data, keys, (sizeof_incomplete / 16) + 1, out);
            //store in a temporary file
            fwrite(out, sizeof(char), sizeof_incomplete + (16 - remainder), f1);
        }
        else
        {
            fread(data, sizeof(char), sizeof_incomplete, f);
            //decrypt 
            inv_cipher(data, keys, (sizeof_incomplete / 16), out);
            //store in a temporary file
            incomplete_data_block.append(out);
            fwrite(out, sizeof(char), incomplete_data_block.size(), f1);
        }
    }

    fclose(f);
    fclose(f1);

    f1 = fopen(template_filepath.c_str(), "rb");
    f = fopen(filepath, "wb");

    sizeof_incomplete = (int)get_size(template_filepath.c_str());

    storing = true;
    SendMessage(hwndprogress, PBM_SETSTEP, (WPARAM)2, 0);
    //store decrypted data
    for (int i = 0; i < no_of_complete_blocks; i++)
    {
        if (complete_task % 2 == 0)
        {
            SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
            complete_task++;
        }      

        fread(data, sizeof(char), COMPLETE_BLOCK, f1);
        fwrite(data, sizeof(char), COMPLETE_BLOCK, f);

        complete_task++;
    }
    if (sizeof_incomplete > 0)
    {
        SendMessage(hwndprogress, PBM_STEPIT, 0, 0);
        complete_task++;

        if ((incomplete_data_block.size() % 16) != 0)
        {
            fread(data, sizeof(char), incomplete_data_block.size(), f1);
            fwrite(data, sizeof(char), incomplete_data_block.size(), f);
        }
        else
        {
            fread(data, sizeof(char), sizeof_incomplete, f1);
            fwrite(data, sizeof(char), sizeof_incomplete, f);
        }
        
    }

    SendMessage(hWndprogress, WM_CLOSE, NULL, 0);
    complete_task = 0;
    fclose(f);
    fclose(f1);
    
    remove(template_filepath.c_str());
}

// AES algorithim cipher
void cipher(char* data, key keys[11], int bunches_of_state, char* out)
{
    int current_index = 0;
    
    for (int i = 0; i < bunches_of_state; i++)
        {
            copy(default_state, defaultstate, 0, "state");     //refresh default_state
            copy(default_state, data, current_index, "state");

            for (int j = 0; j < 11; j++)   //11 rounds
            {
                encrypt(default_state, keys[j], j);
            }
            copy(default_state, out, current_index, "data");
            current_index += 16;
        }   
}

void inv_cipher(char* data, key keys[11], int bunches_of_cipher_text, char* out)
{
    int current_index = 0;

    for (int i = 0; i < bunches_of_cipher_text; i++)
    {
        copy(default_state, defaultstate, 0, "state");     //refresh default_state
        copy(default_state, data, current_index, "state");

        for (int j = (11 - 1); j >= 0; j--) // 11 rounds
        {
            dencrypt(default_state, keys[j], j);
        }
        copy(default_state, out, current_index, "data");
        current_index += 16;
    }
}

//encrypt state
void encrypt(byte state[4][4], byte round_key[4][4], int round)
{  
    if (round == 0)
        AddRoundKey(state, round_key);
    else if (round == 10)
    {
        substitute_bytes(state);
        shiftrows(state);
        AddRoundKey(state, round_key);
    }
    else 
    {
        substitute_bytes(state);
        shiftrows(state);
        MixColumns(state);
        AddRoundKey(state, round_key);
    }
    
}

//decrypt cipher text
void dencrypt(byte cipher_text[4][4], byte round_key[4][4], int round)
{ 
    if (round == 0)
        AddRoundKey(cipher_text, round_key);
    else if (round == 10)
    {
        AddRoundKey(cipher_text, round_key);
        Invshiftrows(cipher_text);
        Inv_substitute_bytes(cipher_text);
    }
    else 
    {
        AddRoundKey(cipher_text, round_key);
        InvMixColumns(cipher_text);
        Invshiftrows(cipher_text);
        Inv_substitute_bytes(cipher_text);
        
     }
   
 }


/*************************************************/
/*
* Substitute byte transformation
*
*/
/*************************************************/

const byte S_BOX[256] =
{
     0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
     0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
     0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
     0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
     0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
     0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
     0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
     0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
     0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
     0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
     0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
     0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
     0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
     0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
     0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
     0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16

};

const byte INVERSE_S_BOX[256] =

{
      0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
      0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
      0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
      0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
      0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
      0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
      0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
      0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
      0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
      0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
      0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
      0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
      0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
      0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
      0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
      0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

void substitute_bytes(byte state[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            state[i][j] = S_BOX[state[i][j]];
     }        
   
}

void Inv_substitute_bytes(byte state[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
             state[i][j] = INVERSE_S_BOX[state[i][j]];
    }
   
}

/*************************************************/
/*
* Shift rows transformation
* 
*/
/*************************************************/

void shiftrows(byte state[4][4])
{
    byte temp;
    //row 1 //1 2 3 4
    temp = state[1][3];
    state[1][3] = state[1][0];
    //1 2 3 1
    state[1][0] = state[1][1];
    //2 - 3 1
    state[1][1] = state[1][2];
    //2 3 - 1
    state[1][2] = temp;
    //2 3 4 1

    //row 2 // 1 2 3 4
    temp = state[2][2];

    state[2][2] = state[2][0];
    // - 2 1 4
    state[2][0] = temp;
    // 3 2 1 4

    temp = state[2][3];
    state[2][3] = state[2][1];
    // 3 - 1 2
    state[2][1] = temp;
    // 3 4 1 2

    //row 3 // 1 2 3 4
    temp = state[3][1];

    state[3][1] = state[3][0];
    // - 1 3 4
    state[3][0] = state[3][3];
    // 4 1 3 -
    state[3][3] = state[3][2];
    // 4 1 - 3
    state[3][2] = temp;
    // 4 1 2 3

}

void Invshiftrows(byte state[4][4])
{
    byte temp;
    //row 1 //2 3 4 1
    temp = state[1][0];
    state[1][0] = state[1][3];
    // 1 3 4 -
    state[1][3] = state[1][2];
    // 1 3 - 4
    state[1][2] = state[1][1];
    // 1 - 3 4
    state[1][1] = temp;
    // 1 2 3 4

    //row 2 // 3 4 1 2
    temp = state[2][0];
    state[2][0] = state[2][2];
    // 1 4 - 2
    state[2][2] = temp;
    // 1 4 3 2
    temp = state[2][1];
    state[2][1] = state[2][3];
    // 1 2 3 -
    state[2][3] = temp;
    // 1 2 3 4

    //row 3 // 4 1 2 3
    temp = state[3][0];
    state[3][0] = state[3][1];
    // 1 - 2 3
    state[3][1] = state[3][2];
    // 1 2 - 3
    state[3][2] = state[3][3];
    // 1 2 3 -
    state[3][3] = temp;
    // 1 2 3 4
}


/*************************************************/
/*
*  Mix columns transformation
*
*/
/*************************************************/

const byte MIX_MATRIX[4][4] = {
                                 {0x02,0x03, 0x01, 0x01},
                                 {0x01,0x02, 0x03, 0x01},
                                 {0x01,0x01, 0x02, 0x03},
                                 {0x03,0x01, 0x01, 0x02},
                              };

const byte INVERSE_MIX_MATRIX[4][4] = {
                                         {0x0e,0x0b, 0x0d, 0x09},
                                         {0x09,0x0e, 0x0b, 0x0d},
                                         {0x0d,0x09, 0x0e, 0x0b},
                                         {0x0b,0x0d, 0x09, 0x0e},
                                      };

//Multiplying a byte by the Galois Field ( Multiplication over Finite Fields GF(2^8) )
byte GFMul(byte Galois_field_byte, byte State_byte) 
{
    byte result = 0;
    byte high_bit;
    for (int counter = 0; counter < 8; counter++)
    {
        if ((State_byte & byte(1)) != 0) //if LSB is active (equivalent to Galois_field_byte '1' in the polynomial of State_byte;
        {
            result ^= Galois_field_byte; // p += Galois_field_byte in the GF(2^8);
        }

        high_bit = (byte)(Galois_field_byte & byte(0x80)); // Galois_field_byte >= 128 bits = 0100 0000
        Galois_field_byte <<= 1;							 // rotate Galois_field_byte left (multiply by x in  GF(2^8);

        if (high_bit != 0)
        {
            //must reduce
            Galois_field_byte ^= 0x1b;  // Galois_field_byte -= 00011011 == mod(x^8 + x^4 + x^3 + x + 1)
        }
        State_byte >>= 1;		// rotate State_byte right (divide by x in  GF(2^8);
    }

    return result;
}

void MixColumns(byte state[4][4])
{
    byte temp[4][4];
    copy(state, temp); //store what is in state in a temporary storage

    byte mixed_state_byte;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mixed_state_byte = 0x00;

            for (int k = 0; k < 4; k++)
            {
                mixed_state_byte ^= GFMul(MIX_MATRIX[j][k], temp[k][i]);
            }

            state[j][i] = mixed_state_byte;
        }
    }
}

void InvMixColumns(byte state[4][4])
{
    byte temp[4][4];
    copy(state, temp);  // store what is in state in a temporary storage

    byte mixed_state_byte;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mixed_state_byte = 0x00;

            for (int k = 0; k < 4; k++)
            {
                mixed_state_byte ^= GFMul(INVERSE_MIX_MATRIX[j][k], temp[k][i]);
            }

            state[j][i] = mixed_state_byte;
        }
    }
}

void store_keys(key keys[11], word expandedkeys[44])
{
    int counter = 0;
    for (int i = 0; i < 11; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                keys[i][k][j] = expandedkeys[counter][k];
            }
            counter++;
        }
    }
}

/*************************************************/
/*
*  Key expansion
*/
/*************************************************/

word round_constants[11] = {
                                    {0x00, 0x00, 0x00, 0x00}, {0x01, 0x00, 0x00, 0x00}, {0x02, 0x00, 0x00, 0x00},
                                    {0x04, 0x00, 0x00, 0x00}, {0x08, 0x00, 0x00, 0x00}, {0x10, 0x00, 0x00, 0x00},
                                    {0x20, 0x00, 0x00, 0x00}, {0x40, 0x00, 0x00, 0x00}, {0x80, 0x00, 0x00, 0x00},
                                    {0x1b, 0x00, 0x00, 0x00}, {0x36, 0x00, 0x00, 0x00}
                            };

void xor_word(word w1, word w2)
{
    for (int j = 0; j < 4; j++)
        w1[j] ^= w2[j];
}

void rotate_word(word w)
{
    byte temp;
    // 1 2 3 4
    temp = w[3];
    w[3] = w[0];
    //1 2 3 1
    w[0] = w[1];
    //2 - 3 1
    w[1] = w[2];
    //2 3 - 1
    w[2] = temp;
    //2 3 4 1

}

void substitute_word(word w)
{
    for (int j = 0; j < 4; j++)
        w[j] = S_BOX[w[j]];
}

void KeyExpansion(byte key[4][4], word expanded_keys[44])
{
    word temp;
    int  i = 0;
    //store the original/cipher key in the first word of the expanded_keys
    while (i < 4)
    {
        for (int j = 0; j < 4; j++)
        {
            expanded_keys[i][j] = key[j][i];
        }

        i++;
    }

    while (i < 44)
    {
        for (int j = 0; j < 4; j++)
            temp[j] = expanded_keys[i - 1][j];

        if (i % 4 == 0)
        {
            rotate_word(temp);
            substitute_word(temp);
            xor_word(temp, round_constants[i / 4]);
        }

        xor_word(temp, expanded_keys[i - 4]);

        for (int j = 0; j < 4; j++)
            expanded_keys[i][j] = temp[j];

        i++;
    }

}

/*************************************************/
/*
*  Add round key
*/
/*************************************************/

void AddRoundKey(byte state[4][4], byte round_key[4][4])
{
    byte rows, columns;

    for (columns = 0; columns < 4; columns++)
    {
        for (rows = 0; rows < 4; rows++)
        {
            state[rows][columns] = state[rows][columns] ^ round_key[rows][columns];
        }
    }
}

/*************************************************/
/*
*  Shared methods
*
*/
/*************************************************/

int64_t get_size(const char* filepath)
{
    FILE* f = fopen(filepath, "rb");
    fseek(f, 0, SEEK_END);
    int64_t size = ftell(f);
    fclose(f);
    return size;
}

void copy(byte arr1[4][4], byte arr2[4][4])
{
    for (int row = 0; row < 4; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            arr2[row][column] = arr1[row][column];
        }

    }
}

void copy(byte arr[4][4], char* data, int index, std::string where)
{
    int counter = index;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (where.compare("state") == 0)
                arr[j][i] = data[counter];
            else
                data[counter] = arr[j][i];

            counter++;
        }
    }
}

void align(byte arr[4][4], byte unaligned_arr[4][4])
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            arr[j][i] = unaligned_arr[i][j];

        }
    }
}

