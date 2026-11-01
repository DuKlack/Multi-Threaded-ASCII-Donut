#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define PI 3.14159265358979323846f
#define NUM_THREADS 2
#define TOTAL_FRAMES 1000

const float theta_spacing = 0.07f;
const float phi_spacing   = 0.02f;
const float R1 = 1.0f;
const float R2 = 2.0f;
const float K2 = 5.0f;

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 40

const float K1 = SCREEN_WIDTH * K2 * 3.0f / (8.0f * (R1 + R2));

typedef struct {
    float A;
    float B;
    float theta_start;
    float theta_end;
    char output[SCREEN_WIDTH][SCREEN_HEIGHT];
    float zbuffer[SCREEN_WIDTH][SCREEN_HEIGHT];
} ThreadData;

void* render_slice(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    float cosA = cosf(data->A), sinA = sinf(data->A);
    float cosB = cosf(data->B), sinB = sinf(data->B);

    memset(data->output, ' ', sizeof(data->output));
    memset(data->zbuffer, 0, sizeof(data->zbuffer));

    for (float theta = data->theta_start; theta < data->theta_end; theta += theta_spacing) {
        float costheta = cosf(theta), sintheta = sinf(theta);

        for (float phi = 0; phi < 2 * PI; phi += phi_spacing) {
            float cosphi = cosf(phi), sinphi = sinf(phi);

            float circlex = R2 + R1 * costheta;
            float circley = R1 * sintheta;

            float x = circlex * (cosB * cosphi + sinA * sinB * sinphi) - circley * cosA * sinB;
            float y = circlex * (sinB * cosphi - sinA * cosB * sinphi) + circley * cosA * cosB;
            float z = K2 + cosA * circlex * sinphi + circley * sinA;
            float ooz = 1.0f / z;

            int xp = (int)(SCREEN_WIDTH / 2 + K1 * ooz * x);
            int yp = (int)(SCREEN_HEIGHT / 2 - K1 * ooz * y);

            if (xp >= 0 && xp < SCREEN_WIDTH && yp >= 0 && yp < SCREEN_HEIGHT) {
                float L = cosphi * costheta * sinB - cosA * costheta * sinphi 
                        - sinA * sintheta + cosB * (cosA * sintheta - costheta * sinA * sinphi);

                if (L > 0) {
                    if (ooz > data->zbuffer[xp][yp]) {
                        data->zbuffer[xp][yp] = ooz;
                        int luminance_index = (int)(L * 8.0f);
                        if (luminance_index > 11) luminance_index = 11;
                        data->output[xp][yp] = ".,-~:;=!*#$@"[luminance_index];
                    }
                }
            }
        }
    }

    pthread_exit(NULL);
}

void render_frame(float A, float B) {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];

    char final_output[SCREEN_WIDTH][SCREEN_HEIGHT];
    float final_zbuffer[SCREEN_WIDTH][SCREEN_HEIGHT];

    memset(final_output, ' ', sizeof(final_output));
    memset(final_zbuffer, 0, sizeof(final_zbuffer));

    float total_theta = 2.0f * PI;
    float slice_size = total_theta / NUM_THREADS;

    for (int t = 0; t < NUM_THREADS; t++) {
        thread_data[t].A = A;
        thread_data[t].B = B;
        thread_data[t].theta_start = t * slice_size;
        thread_data[t].theta_end = (t == NUM_THREADS - 1) ? total_theta : (t + 1) * slice_size;

        pthread_create(&threads[t], NULL, render_slice, (void*)&thread_data[t]);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);

        for (int i = 0; i < SCREEN_WIDTH; i++) {
            for (int j = 0; j < SCREEN_HEIGHT; j++) {
                if (thread_data[t].zbuffer[i][j] > final_zbuffer[i][j]) {
                    final_zbuffer[i][j] = thread_data[t].zbuffer[i][j];
                    final_output[i][j] = thread_data[t].output[i][j];
                }
            }
        }
    }

    // Dump frame output to terminal
    printf("\x1b[H");
    for (int j = 0; j < SCREEN_HEIGHT; j++) {
        for (int i = 0; i < SCREEN_WIDTH; i++) {
            putchar(final_output[i][j]);
        }
        putchar('\n');
    }
}

int main() {
    float A = 0.0f;
    float B = 0.0f;

    struct timespec start_time, end_time;

    // Clear terminal screen before starting
    printf("\x1b[2J");

    // Start wall-clock timer
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        render_frame(A, B);
        A += 0.04f;
        B += 0.02f;
    }

    // Stop wall-clock timer
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    // Calculate elapsed time in seconds
    double elapsed_sec = (end_time.tv_sec - start_time.tv_sec) +
                         (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    double fps = TOTAL_FRAMES / elapsed_sec;

    // Print benchmark metrics below rendered output
    printf("\n=== Performance Metrics ===\n");
    printf("Rendered Frames: %d\n", TOTAL_FRAMES);
    printf("Total Time:      %.4f seconds\n", elapsed_sec);
    printf("Average Time/Frame: %.4f ms\n", (elapsed_sec / TOTAL_FRAMES) * 1000.0);
    printf("Average FPS:     %.2f\n", fps);

    return 0;
}