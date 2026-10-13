#include <stdio.h>

#define MAX_FRAMES 50

static int send_frame(int frame, int lost_interval, int lost_frames[]) {
    printf("Peer A sends Frame %d", frame);
    if (lost_interval > 0 && (frame + 1) % lost_interval == 0 && !lost_frames[frame]) {
        lost_frames[frame] = 1;
        printf(" -> lost\n");
        return 0;
    }
    printf(" -> received by Peer B\n");
    return 1;
}

static void go_back_n(int count, int size, int lost) {
    int first = 0;
    int total_transmissions = 0;
    int lost_frames[MAX_FRAMES] = {0};

    printf("\n--- Go-Back-N ---\n");
    while (first < count) {
        int last = first + size;
        int error = -1;
        int frame;

        if (last > count) last = count;
        for (frame = first; frame < last; frame++) {
            if (error != -1) {
                printf("Peer B discards Frame %d (out of order)\n", frame);
            } else if (!send_frame(frame, lost, lost_frames)) {
                total_transmissions++;
                error = frame;
            } else {
                total_transmissions++;
            }
        }
        if (error != -1) {
            printf("Peer B sends ACK %d\n", error);
            printf("Retransmitting from Frame %d\n\n", error);
            first = error;
        } else {
            printf("Peer B sends ACK %d\n\n", last);
            first = last;
        }
    }
    printf("All frames received.\n");
    printf("Total transmissions: %d\n", total_transmissions);
}

static void selective_repeat(int count, int size, int lost) {
    int received[MAX_FRAMES] = {0};    int lost_frames[MAX_FRAMES] = {0};    int first = 0;
    int total_transmissions = 0;

    printf("\n--- Selective Repeat ---\n");
    while (first < count) {
        int last = first + size;
        int frame;

        if (last > count) last = count;
        for (frame = first; frame < last; frame++) {
            if (!received[frame] && send_frame(frame, lost, lost_frames)) {
                received[frame] = 1;
                total_transmissions++;
                printf("Peer B sends ACK %d\n", frame);
            } else if (!received[frame]) {
                total_transmissions++;
            }
        }
        while (first < count && received[first]) first++;
        if (first < last) printf("Only the lost frame is retransmitted.\n\n");
    }
    printf("All frames received.\n");
    printf("Total transmissions: %d\n", total_transmissions);
}

int main(void) {
    int mode, count, size, lost;

    printf("===== Sliding Window Protocol =====\n");
    printf("1. Go-Back-N\n2. Selective Repeat\nChoose protocol: ");
    scanf("%d", &mode);
    printf("Number of frames (1-%d): ", MAX_FRAMES);
    scanf("%d", &count);
    printf("Window size: ");
    scanf("%d", &size);
    printf("Every nth frame is lost (0 for no loss): ");
    scanf("%d", &lost);

    if (mode < 1 || mode > 2 || count < 1 || count > MAX_FRAMES ||
        size < 1 || size > count || lost < 0) {
        printf("Invalid input.\n");
    } else if (mode == 1) {
        go_back_n(count, size, lost);
    } else {
        selective_repeat(count, size, lost);
    }
    return 0;
}