#include <stdio.h>

#define MAX_ROUNDS 1000

static void simulate_tcp(int rounds, int loss_round)
{
    double congestion_window = 1.0;
    double slow_start_threshold = 16.0;

    printf("TCP (AIMD) simulation\n");
    printf("Round\tCongestion window\tEvent\n");
    for (int round = 1; round <= rounds; round++) {
        const char *event = "ACK";

        if (round == loss_round) {
            slow_start_threshold = congestion_window / 2.0;
            if (slow_start_threshold < 1.0) {
                slow_start_threshold = 1.0;
            }
            congestion_window = 1.0;
            event = "LOSS: timeout, slow start restarted";
        } else if (congestion_window < slow_start_threshold) {
            congestion_window *= 2.0;
            event = "slow start";
        } else {
            congestion_window += 1.0;
            event = "congestion avoidance (additive increase)";
        }

        printf("%d\t%.2f\t\t%s\n", round, congestion_window, event);
    }
}

static void simulate_udp(int rounds, double packets_per_round, int loss_round)
{
    double total_sent = 0.0;
    double total_lost = 0.0;

    printf("\nUDP (constant-rate, no built-in congestion control) simulation\n");
    printf("Round\tSent\tLost\tEvent\n");
    for (int round = 1; round <= rounds; round++) {
        double lost = round == loss_round ? packets_per_round * 0.50 : 0.0;
        total_sent += packets_per_round;
        total_lost += lost;
        printf("%d\t%.2f\t%.2f\t%s\n", round, packets_per_round, lost,
               lost > 0.0 ? "network congestion; sender rate unchanged" : "no feedback-based reduction");
    }

    printf("\nUDP summary: sent %.2f packets, lost %.2f packets, loss rate %.2f%%\n",
           total_sent, total_lost, total_sent == 0.0 ? 0.0 : (total_lost / total_sent) * 100.0);
}

int main(void)
{
    int rounds;
    int loss_round;
    double udp_rate;

    printf("Enter number of simulation rounds (1-%d): ", MAX_ROUNDS);
    if (scanf("%d", &rounds) != 1 || rounds < 1 || rounds > MAX_ROUNDS) {
        fprintf(stderr, "Invalid number of rounds.\n");
        return 1;
    }

    printf("Enter TCP loss round (0 for no loss): ");
    if (scanf("%d", &loss_round) != 1 || loss_round < 0 || loss_round > rounds) {
        fprintf(stderr, "Invalid loss round.\n");
        return 1;
    }

    printf("Enter UDP packets sent per round: ");
    if (scanf("%lf", &udp_rate) != 1 || udp_rate < 0.0) {
        fprintf(stderr, "Invalid UDP rate.\n");
        return 1;
    }

    simulate_tcp(rounds, loss_round);
    simulate_udp(rounds, udp_rate, loss_round);

    printf("\nAnalysis:\n");
    printf("TCP adapts its sending window using feedback: it increases gradually and reduces after loss.\n");
    printf("UDP provides no congestion-control mechanism, so its configured sending rate remains constant.\n");

    return 0;
}
