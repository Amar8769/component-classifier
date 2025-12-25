#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#define MAX_LINE 100

// Load character representation from file
int ch_load(FILE *p, wchar_t *wchars, char output[15][11])
{
    char line[MAX_LINE];
    rewind(p);

    char label[40];
    char utf8_char[10];

    int len = WideCharToMultiByte(CP_UTF8, 0, wchars, -1, utf8_char, sizeof(utf8_char), NULL, NULL); // convert wide char to UTF-8, windows library
    if (len <= 0)
    {
        wprintf(L"Failed to convert char to UTF-8: %lc\n", wchars[0]); // debug
        return 0;
    }

    utf8_char[len] = '\0';
    sprintf(label, "TEXT: %s", utf8_char);

    wprintf(L"Looking for: %hs\n", label);

    while (fgets(line, sizeof(line), p))
    {
        if (strncmp(line, label, strlen(label)) == 0)
        {
            int row = 0;
            while (fgets(line, sizeof(line), p))
            {
                if (strncmp(line, "TEXT:", 5) == 0)
                {
                    fseek(p, -strlen(line), SEEK_CUR);
                    break;
                }

                line[strcspn(line, "\r\n")] = '\0';

                if (row < 15)
                {
                    strncpy(output[row], line, 11);
                    output[row][11] = '\0';
                    row++;
                }
            }

            return 1;
        }
    }

    printf("Character not found in file: %hs\n", label); // debug
    return 0;
}

// Build grapheme cluster (emoji or Hindi)
int get_next_cluster(wchar_t *wstr, int len, int start_index, wchar_t *cluster_out)
{
    int i = start_index;
    int cluster_len = 0;

    // Handle emoji surrogate pairs
    if (i + 1 < len &&
        wstr[i] >= 0xD800 && wstr[i] <= 0xDBFF &&
        wstr[i + 1] >= 0xDC00 && wstr[i + 1] <= 0xDFFF)
    {
        cluster_out[cluster_len++] = wstr[i++];
        cluster_out[cluster_len++] = wstr[i++];
    }
    else
    {
        // Add first character
        cluster_out[cluster_len++] = wstr[i++];

        // Handle Devanagari halant clusters
        // note - halant is 0x094D and consonants are 0x0905-0x0939
        while (i + 1 < len && wstr[i] == 0x094D)
        {
            cluster_out[cluster_len++] = wstr[i++]; // halant
            cluster_out[cluster_len++] = wstr[i++]; // next consonant
        }
    }

    cluster_out[cluster_len] = L'\0';
    return cluster_len;
}

int main()
{
    setlocale(LC_ALL, ".UTF-8"); // allow UTF-8 input/output

    // input and output handles
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD written, charsRead; // store number of characters written/read

    wchar_t wstr[100];
    char char_matrix[100][15][11];
    int char_width[100], char_height[100];
    int scale_x[100], scale_y[100];

    char custom_chars[100] = {0};
    int count = 0;

    FILE *p = fopen("alpha.txt", "r");
    if (!p)
    {
        WriteConsoleW(hOut, L"File not found.\n", 17, &written, NULL);
        return 1;
    }

    WriteConsoleW(hOut, L"default: ", 9, &written, NULL);
    char ch;
    scanf(" %c", &ch);
    getchar();

    WriteConsoleW(hOut, L"Enter text: ", 13, &written, NULL);
    FlushConsoleInputBuffer(hIn);
    if (ReadConsoleW(hIn, wstr, 100, &charsRead, NULL))
    {
        if (charsRead > 0 && wstr[charsRead - 1] == L'\n')
            wstr[charsRead - 1] = L'\0';
        if (charsRead > 1 && wstr[charsRead - 2] == L'\r')
            wstr[charsRead - 2] = L'\0';

        WriteConsoleW(hOut, L"\nReceived input: ", 18, &written, NULL); // note - debug, delete later
        WriteConsoleW(hOut, wstr, wcslen(wstr), &written, NULL);
        WriteConsoleW(hOut, L"\n\n", 2, &written, NULL);
    }
    else
    {
        WriteConsoleW(hOut, L"Error reading input.\n", 22, &written, NULL);
        return 1;
    }

    WriteConsoleW(hOut, L"width: ", 7, &written, NULL);
    int width;
    scanf("%d", &width);
    WriteConsoleW(hOut, L"height: ", 8, &written, NULL);
    int height;
    scanf("%d", &height);
    getchar();

    WriteConsoleW(hOut, L"Enter custom (e.g. 1 - $, 5 - d): ", 34, &written, NULL);
    char override[200];
    fgets(override, sizeof(override), stdin);

    int pos;
    char sym;
    char *token = strtok(override, ",");
    while (token)
    {
        if (sscanf(token, " %d - %c", &pos, &sym) == 2 && pos > 0 && pos <= wcslen(wstr))
        {
            custom_chars[pos - 1] = sym;
        }
        token = strtok(NULL, ",");
    }

    int i = 0;
    int len = wcslen(wstr);
    while (i < len)
    {
        wchar_t cluster[10] = {0};
        int cluster_len = get_next_cluster(wstr, len, i, cluster);

        wprintf(L"Loading cluster: %ls\n", cluster);

        if (!ch_load(p, cluster, char_matrix[count]))
        {
            for (int j = 0; j < 15; j++)
                strcpy(char_matrix[count][j], "     ");
        }

        int actual_width = 0, actual_height = 0;
        for (int j = 0; j < 15; j++)
        {
            int is_blank = 1;
            int row_len = strlen(char_matrix[count][j]);
            for (int k = 0; k < row_len; k++)
            {
                if (char_matrix[count][j][k] == '#')
                {
                    is_blank = 0;
                    break;
                }
            }

            if (!is_blank)
            {
                actual_height = j + 1;
            }

            if (row_len > actual_width)
                actual_width = row_len;
        }

        if (actual_width == 0)
            actual_width = 1;
        if (actual_height == 0)
            actual_height = 1;

        char_width[count] = actual_width;
        char_height[count] = actual_height;

        scale_x[count] = width / actual_width;
        scale_y[count] = height / actual_height;

        if (scale_x[count] < 1)
            scale_x[count] = 1;
        if (scale_y[count] < 1)
            scale_y[count] = 1;

        count++;
        i += cluster_len;
    }

    int max_scaled_height = 0;
    for (int i = 0; i < count; i++)
    {
        int scaled_height = char_height[i] * scale_y[i];
        if (scaled_height > max_scaled_height)
            max_scaled_height = scaled_height;
    }

    for (int row = 0; row < max_scaled_height; row++)
    {
        for (int i = 0; i < count; i++)
        {
            int orig_row = row / scale_y[i];
            if (orig_row >= char_height[i])
            {
                for (int col = 0; col < char_width[i]; col++)
                    for (int sx = 0; sx < scale_x[i]; sx++)
                        printf(" ");
            }
            else
            {
                for (int col = 0; col < char_width[i]; col++)
                {
                    char pixel = char_matrix[i][orig_row][col];
                    for (int sx = 0; sx < scale_x[i]; sx++)
                        printf("%c", pixel == '#' ? (custom_chars[i] ? custom_chars[i] : ch) : ' ');
                }
            }
            printf("  ");
        }
        printf("\n");
    }

    WriteConsoleW(hOut, L"\nDisplay complete.\n", 19, &written, NULL); // note - debug, delete later
    fclose(p);
    return 0;
}
