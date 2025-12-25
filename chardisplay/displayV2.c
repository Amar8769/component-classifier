#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#define MAX_LINE 100

int ch_load(FILE *p, wchar_t *wchars, char output[15][11])
{
    char line[MAX_LINE];
    rewind(p);
    char label[40];
    char utf8_char[10];

    int len = WideCharToMultiByte(CP_UTF8, 0, wchars, -1, utf8_char, sizeof(utf8_char), NULL, NULL);
    if (len <= 0)
        return 0;

    utf8_char[len] = '\0';
    sprintf(label, "TEXT: %s", utf8_char);

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

    return 0;
}

int get_next_cluster(wchar_t *wstr, int len, int start_index, wchar_t *cluster_out)
{
    int i = start_index;
    int cluster_len = 0;

    if (i + 1 < len &&
        wstr[i] >= 0xD800 && wstr[i] <= 0xDBFF &&
        wstr[i + 1] >= 0xDC00 && wstr[i + 1] <= 0xDFFF)
    {
        cluster_out[cluster_len++] = wstr[i++];
        cluster_out[cluster_len++] = wstr[i++];
    }
    else
    {
        cluster_out[cluster_len++] = wstr[i++];
        while (i + 1 < len && wstr[i] == 0x094D)
        {
            cluster_out[cluster_len++] = wstr[i++];
            cluster_out[cluster_len++] = wstr[i++];
        }
    }

    cluster_out[cluster_len] = L'\0';
    return cluster_len;
}

int wmain(int argc, wchar_t *argv[])
{
    setlocale(LC_ALL, ".UTF-8");

    if (argc < 5)
    {
        fwprintf(stderr, L"Usage: %s <text> <width> <height> <default_char> [optional: index - char ...]\n", argv[0]);
        return 1;
    }

    wchar_t *text = argv[1];
    int width = _wtoi(argv[2]);
    int height = _wtoi(argv[3]);
    char default_char = argv[4][0];

    char custom_chars[100] = {0};
    for (int i = 5; i < argc; i++)
    {
        int index;
        char symbol;
        if (swscanf(argv[i], L"%d - %c", &index, &symbol) == 2 && index > 0 && index <= wcslen(text))
        {
            custom_chars[index - 1] = symbol;
        }
    }

    FILE *p = fopen("alpha.txt", "r");
    if (!p)
        return 1;

    FILE *out = fopen("matrix_output.txt", "w");
    if (!out)
    {
        fclose(p);
        return 1;
    }

    wchar_t *wstr = text;
    char char_matrix[100][15][11];
    int char_width[100], char_height[100];
    int scale_x[100], scale_y[100];
    int count = 0, i = 0;
    int len = wcslen(wstr);

    while (i < len)
    {
        wchar_t cluster[10] = {0};
        int cluster_len = get_next_cluster(wstr, len, i, cluster);

        if (!ch_load(p, cluster, char_matrix[count]))
        {
            wchar_t part1[10] = {0}, part2[10] = {0};
            int len1 = (cluster[1] == 0x094D) ? 2 : 1;
            wcsncpy(part1, cluster, len1);
            wcscpy(part2, cluster + len1);

            char temp[15][11] = {0};
            int got_base = ch_load(p, part1, char_matrix[count]);
            int got_matra = ch_load(p, part2, temp);

            if (got_base && got_matra)
            {
                for (int row = 0; row < 15; row++)
                    for (int col = 0; col < 11; col++)
                        if (temp[row][col] == '#')
                            char_matrix[count][row][col] = '#';
            }
            else
            {
                for (int j = 0; j < 15; j++)
                    strcpy(char_matrix[count][j], "           ");
            }
        }

        int actual_width = 1, actual_height = 1;
        for (int j = 0; j < 15; j++)
        {
            int blank = 1;
            for (int k = 0; k < strlen(char_matrix[count][j]); k++)
            {
                if (char_matrix[count][j][k] == '#')
                {
                    blank = 0;
                    break;
                }
            }
            if (!blank)
                actual_height = j + 1;
            if (strlen(char_matrix[count][j]) > actual_width)
                actual_width = strlen(char_matrix[count][j]);
        }

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
                        fprintf(out, " ");
            }
            else
            {
                for (int col = 0; col < char_width[i]; col++)
                {
                    char pixel = char_matrix[i][orig_row][col];
                    for (int sx = 0; sx < scale_x[i]; sx++)
                        fprintf(out, "%c", pixel == '#' ? (custom_chars[i] ? custom_chars[i] : default_char) : ' ');
                }
            }
            fprintf(out, "  ");
        }
        fprintf(out, "\n");
    }

    fclose(p);
    fclose(out);
    return 0;
}

// Main for GCC (ASCII-only) to simulate wmain
int main(int argc, char *argv[])
{
    wchar_t w_argv[20][100];
    wchar_t *wargs[20];

    for (int i = 0; i < argc; i++)
    {
        MultiByteToWideChar(CP_UTF8, 0, argv[i], -1, w_argv[i], 100);
        wargs[i] = w_argv[i];
    }

    return wmain(argc, wargs);
}
