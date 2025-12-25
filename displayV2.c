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
    {
        wprintf(L"Failed to convert char to UTF-8: %lc\n", wchars[0]);
        return 0;
    }

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

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, ".UTF-8");

    if (argc < 5)
    {
        printf("Usage: %s <text> <width> <height> <default_char> [custom_overrides]\n", argv[0]);
        return 1;
    }

    wchar_t wstr[100];
    mbstowcs(wstr, argv[1], 100);
    int width = atoi(argv[2]);
    int height = atoi(argv[3]);
    char ch = argv[4][0];
    char *override = (argc >= 6) ? argv[5] : "";

    FILE *p = fopen("alpha.txt", "r");
    if (!p)
    {
        printf("File not found.\n");
        return 1;
    }

    char custom_chars[100] = {0};
    int count = 0;

    int pos;
    char sym;
    char *token = strtok(override, ",");
    while (token)
    {
        if (sscanf(token, " %d - %c", &pos, &sym) == 2 && pos > 0 && pos < 100)
        {
            custom_chars[pos - 1] = sym;
        }
        token = strtok(NULL, ",");
    }

    char char_matrix[100][15][11];
    int char_width[100], char_height[100];
    int scale_x[100], scale_y[100];

    int i = 0;
    int len = wcslen(wstr);
    while (i < len)
    {
        wchar_t cluster[10] = {0};
        int cluster_len = get_next_cluster(wstr, len, i, cluster);

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
                actual_height = j + 1;
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

    char output_matrix[500][1000]; // Large buffer for matrix lines
    int current_line = 0;

    for (int row = 0; row < max_scaled_height; row++)
    {
        char line_buffer[1000] = "";
        for (int i = 0; i < count; i++)
        {
            int orig_row = row / scale_y[i];
            if (orig_row >= char_height[i])
            {
                for (int col = 0; col < char_width[i]; col++)
                {
                    for (int sx = 0; sx < scale_x[i]; sx++)
                    {
                        strcat(line_buffer, " ");
                    }
                }
            }
            else
            {
                for (int col = 0; col < char_width[i]; col++)
                {
                    char pixel = char_matrix[i][orig_row][col];
                    for (int sx = 0; sx < scale_x[i]; sx++)
                    {
                        char to_print = (pixel == '#') ? (custom_chars[i] ? custom_chars[i] : ch) : ' ';
                        char str[2] = {to_print, '\0'};
                        strcat(line_buffer, str);
                    }
                }
            }
            strcat(line_buffer, "  ");
        }

        printf("%s\n", line_buffer);
        strncpy(output_matrix[current_line++], line_buffer, sizeof(output_matrix[0]));
    }

    fclose(p);

    // Write output to file
    FILE *outf = fopen("matrix_output.txt", "w");
    if (outf)
    {
        for (int i = 0; i < current_line; i++)
        {
            fprintf(outf, "%s\n", output_matrix[i]);
        }
        fclose(outf);
    }
    else
    {
        printf("Failed to open output file.\n");
    }

    return 0;
}
