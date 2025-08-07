/***************************************************************************
 * Description: convert pcm to wav
 * version: 0.1.0
 * Author: Panda-Young
 * Date: 2025-08-07 21:36:56
 * Copyright (c) 2025 by Panda-Young, All Rights Reserved.
 **************************************************************************/

#include "log.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH 260

// WAV file header structure
typedef struct {
    // RIFF header
    char riff_header[4]; // "RIFF"
    int file_size;       // File size - 8
    char wave_header[4]; // "WAVE"

    // Format chunk
    char fmt_header[4];    // "fmt "
    int fmt_chunk_size;    // Format chunk size (16 for PCM)
    short audio_format;    // Audio format (1 for PCM)
    short channels;        // Number of channels
    int sample_rate;       // Sample rate
    int byte_rate;         // Byte rate (sample_rate * channels * bits_per_sample / 8)
    short block_align;     // Block align (channels * bits_per_sample / 8)
    short bits_per_sample; // Bits per sample

    // Data chunk
    char data_header[4]; // "data"
    int data_chunk_size; // Data chunk size
} wav_header_t;

// Define supported sample rates
static const int supported_sample_rates[] = {
    192000, 176400, 96000, 88200, 64000, 48000, 44100,
    32000, 22050, 16000, 11025, 8000, 6000};
static const int num_supported_sample_rates = sizeof(supported_sample_rates) / sizeof(supported_sample_rates[0]);

// Define long options for command line parsing
static struct option long_options[] = {
    {"output", required_argument, 0, 'o'},
    {"sample_rate", required_argument, 0, 's'},
    {"bit_width", required_argument, 0, 'b'},
    {"channel", required_argument, 0, 'c'},
    {"format", required_argument, 0, 'f'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}};

/**
 * @brief Display help message for the program usage
 */
void display_help()
{
    printf("Usage: pcm_to_wav <input_file> -s <sample_rate> -b <bit_width> -c <channel> [options]\n");
    printf("\n");
    printf("Required Options:\n");
    printf("  -s, --sample_rate <rate>  Sample rate (supported: 192000, 176400, 96000, 88200, 64000, 48000, 44100, 32000, 22050, 16000, 11025, 8000, 6000)\n");
    printf("  -b, --bit_width <width>   Bit width (e.g., 16)\n");
    printf("  -c, --channel <number>    Channel number (e.g., 2)\n");
    printf("  -f, --format <format>     Audio format (1 for PCM, 3 for IEEE_FLOAT)\n");
    printf("\n");
    printf("Optional Options:\n");
    printf("  -o, --output <file>       Output WAV file path\n");
    printf("  -h, --help                Display this help message\n");
    printf("\n");
    printf("Example:\n");
    printf("  pcm_to_wav c:\\test.pcm -s 48000 -b 16 -c 2 -f 1\n");
    printf("  pcm_to_wav input.pcm -o output.wav -s 44100 -b 16 -c 1 -f 1\n");
}
/**
 * @brief Check if input file has a supported extension
 *
 * @param filename Input filename
 * @return 0 if supported, -1 if not supported
 */
int valid_file_format(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot) {
        return -1; // No extension
    }

    if (strcmp(dot, ".pcm") == 0 || strcmp(dot, ".raw") == 0) {
        return 0; // Supported format
    }

    return -1; // Unsupported format
}

/**
 * @brief Check if the sample rate is supported
 *
 * @param sample_rate Sample rate to check
 * @return 0 if supported, -1 if not supported
 */
int valid_sample_rate(int sample_rate)
{
    for (int i = 0; i < num_supported_sample_rates; i++) {
        if (sample_rate == supported_sample_rates[i]) {
            return 0;
        }
    }
    return -1;
}

/**
 * @brief Validate parameters based on format requirements
 *
 * @param format Audio format
 * @param bit_width Bit width
 * @return 0 if valid, -1 if invalid
 */
int valid_audio_format_and_bit_width(int bit_width, int format)
{
    if (format == 3 && bit_width != 32 && bit_width != 64) {
        LOGE("For IEEE_FLOAT format (-f 3), bit width (-b) must be 32 or 64");
        return -1;
    }

    if (format == 1 && bit_width != 8 && bit_width != 16 && bit_width != 24 && bit_width != 32) {
        LOGE("For PCM format (-f 1), bit width (-b) must be 8, 16, 24, or 32");
        return -1;
    }
    return 0;
}
/**
 * @brief Print supported sample rates
 */
void print_supported_sample_rates()
{
    printf("Supported sample rates: ");
    for (int i = 0; i < num_supported_sample_rates; i++) {
        printf("%d", supported_sample_rates[i]);
        if (i < num_supported_sample_rates - 1) {
            printf(", ");
        }
    }
    printf("\n");
}

/**
 * @brief Initialize WAV header with specified parameters
 *
 * @param header Pointer to WAV header structure
 * @param sample_rate Sample rate
 * @param bit_width Bit width
 * @param channel Channel count
 * @param format Audio format (1 for PCM, 3 for IEEE_FLOAT)
 */
void init_wav_header(wav_header_t *header, int sample_rate, int bit_width, int channel, int format)
{
    // Initialize RIFF header
    memcpy(header->riff_header, "RIFF", 4);
    header->file_size = 0; // Will be updated later

    // Initialize WAVE header
    memcpy(header->wave_header, "WAVE", 4);

    // Initialize format chunk
    memcpy(header->fmt_header, "fmt ", 4);
    header->fmt_chunk_size = 16;   // Size of format chunk for PCM
    header->audio_format = format; // Audio format (1 for PCM, 3 for IEEE_FLOAT)
    header->channels = channel;
    header->sample_rate = sample_rate;
    header->bits_per_sample = bit_width;
    header->byte_rate = sample_rate * channel * bit_width / 8;
    header->block_align = channel * bit_width / 8;

    // Initialize data chunk
    memcpy(header->data_header, "data", 4);
    header->data_chunk_size = 0; // Will be updated later
}

/**
 * @brief Parse command line arguments
 *
 * @param argc Argument count
 * @param argv Argument values
 * @param input_file Buffer to store input file path
 * @param output_file Buffer to store output file path
 * @param sample_rate Pointer to store sample rate value
 * @param bit_width Pointer to store bit width value
 * @param channel Pointer to store channel number value
 * @param format Pointer to store audio format value
 * @return 0 on success, -1 on error
 */
int parse_command_line(int argc, char **argv,
                       char *input_file, char *output_file,
                       int *sample_rate, int *bit_width, int *channel, int *format)
{
    int opt;
    int option_index = 0;

    // Flags to check if required parameters are provided
    int sample_rate_set = 0;
    int bit_width_set = 0;
    int channel_set = 0;
    int format_set = 0;

    // Flag to check if input file is provided
    int input_file_set = 0;

    // Parse command line options
    while ((opt = getopt_long(argc, argv, "o:s:b:c:f:h", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'o':
            // Set output file path
            strncpy(output_file, optarg, MAX_PATH - 1);
            output_file[MAX_PATH - 1] = '\0';
            break;
        case 's':
            // Set sample rate
            *sample_rate = atoi(optarg);
            sample_rate_set = 1;
            break;
        case 'b':
            // Set bit width
            *bit_width = atoi(optarg);
            bit_width_set = 1;
            break;
        case 'c':
            // Set channel number
            *channel = atoi(optarg);
            channel_set = 1;
            break;
        case 'f':
            // Set audio format
            *format = atoi(optarg);
            format_set = 1;
            break;
        case 'h':
            // Display help and exit
            display_help();
            exit(0);
        default:
            // Unknown option, display help and return error
            display_help();
            return -1;
        }
    }

    // Handle non-option arguments (input file)
    if (optind < argc) {
        strncpy(input_file, argv[optind], MAX_PATH - 1);
        input_file[MAX_PATH - 1] = '\0';
        input_file_set = 1;
    }

    // Check if input file is provided
    if (!input_file_set) {
        LOGE("Input file is required");
        display_help();
        return -1;
    }

    // Check if input file format is supported
    if (valid_file_format(input_file) != 0) {
        LOGE("Unsupported input file format. Only .pcm and .raw files are supported");
        return -1;
    }

    // Check if all required parameters are provided
    if (!sample_rate_set) {
        LOGE("Sample rate (-s) is required");
        display_help();
        return -1;
    }

    // Validate sample rate
    if (valid_sample_rate(*sample_rate) != 0) {
        LOGE("Unsupported sample rate: %d", *sample_rate);
        print_supported_sample_rates();
        return -1;
    }

    if (!bit_width_set) {
        LOGE("Bit width (-b) is required");
        display_help();
        return -1;
    }

    if (!channel_set) {
        LOGE("Channel number (-c) is required");
        display_help();
        return -1;
    }

    if (!format_set) {
        LOGE("Audio format (-f) is required");
        display_help();
        return -1;
    }

    if (valid_audio_format_and_bit_width(*bit_width, *format) != 0) {
        LOGE("Unsupported audio format or bit width: %d, %d", *bit_width, *format);
        return -1;
    }

    // If output file is not specified, generate it from input file
    if (strlen(output_file) == 0) {
        strncpy(output_file, input_file, MAX_PATH - 1);
        output_file[MAX_PATH - 1] = '\0';
        // Replace .pcm extension with .wav
        char *dot = strrchr(output_file, '.');
        if (dot) {
            strcpy(dot, ".wav");
        } else {
            strcat(output_file, ".wav");
        }
    }

    return 0;
}

/**
 * @brief Convert PCM file to WAV file
 *
 * @param input_file Path to input PCM file
 * @param output_file Path to output WAV file
 * @param sample_rate Sample rate
 * @param bit_width Bit width
 * @param channel Channel count
 * @param format Audio format
 * @return 0 on success, -1 on error
 */
int convert_pcm_to_wav(const char *input_file, const char *output_file,
                       int sample_rate, int bit_width, int channel, int format)
{
    FILE *fp_pcm = NULL;
    FILE *fp_wav = NULL;
    wav_header_t header;
    char *buffer = NULL;
    size_t bytes_read;
    size_t file_size;
    const size_t buffer_size = 1024;

    // Open input PCM file
    fp_pcm = fopen(input_file, "rb");
    if (!fp_pcm) {
        LOGE("Cannot open input file %s. Due to \"%s\"", input_file, strerror(errno));
        return -1;
    }

    // Open output WAV file
    fp_wav = fopen(output_file, "wb");
    if (!fp_wav) {
        LOGE("Cannot create output file %s. Due to \"%s\"", output_file, strerror(errno));
        fclose(fp_pcm);
        return -1;
    }

    // Initialize WAV header
    init_wav_header(&header, sample_rate, bit_width, channel, format);

    // Write WAV header (will be updated later)
    fwrite(&header, sizeof(wav_header_t), 1, fp_wav);

    // Allocate buffer for data transfer
    buffer = (char *)malloc(buffer_size);
    if (!buffer) {
        LOGE("Memory allocation failed");
        fclose(fp_pcm);
        fclose(fp_wav);
        return -1;
    }

    // Copy PCM data to WAV file
    while ((bytes_read = fread(buffer, 1, buffer_size, fp_pcm)) > 0) {
        fwrite(buffer, 1, bytes_read, fp_wav);
    }

    // Get the size of PCM data
    file_size = ftell(fp_wav) - sizeof(wav_header_t);

    // Update WAV header with correct file sizes
    header.file_size = sizeof(wav_header_t) - 8 + file_size;
    header.data_chunk_size = file_size;

    // Write updated header to the beginning of the file
    fseek(fp_wav, 0, SEEK_SET);
    fwrite(&header, sizeof(wav_header_t), 1, fp_wav);

    // Clean up
    free(buffer);
    fclose(fp_pcm);
    fclose(fp_wav);

    LOGI("Successfully converted %s to %s", input_file, output_file);
    LOGI("File size: %zu bytes", file_size);
    LOGI("Audio format: %d", format);

    return 0;
}

/**
 * @brief Main function to process command line arguments and convert PCM to WAV
 *
 * @param argc Argument count
 * @param argv Argument values
 * @return 0 on success, -1 on error
 */
int main(int argc, char **argv)
{
    char input_file[MAX_PATH] = {0};
    char output_file[MAX_PATH] = {0};
    int sample_rate, bit_width, channel, format;

    // Check if any arguments are provided
    if (argc < 2) {
        display_help();
        return 0;
    }

    // Parse command line arguments
    if (parse_command_line(argc, argv, input_file, output_file,
                           &sample_rate, &bit_width, &channel, &format) != 0) {
        return -1;
    }

    // Display parsed parameters
    LOGI("Input file: %s", input_file);
    LOGI("Output file: %s", output_file);
    LOGI("Sample rate: %d Hz", sample_rate);
    LOGI("Bit width: %d bits", bit_width);
    LOGI("Channel number: %d", channel);
    LOGI("Audio format: %d", format);

    // Convert PCM to WAV
    if (convert_pcm_to_wav(input_file, output_file, sample_rate, bit_width, channel, format) != 0) {
        LOGE("Failed to convert PCM to WAV");
        return -1;
    }

    return 0;
}
