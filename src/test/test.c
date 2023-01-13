
#include <stdio.h>
#include "minunit.h"
#include "watchpoint.h"


MU_TEST(test_watchpoint) {
	init_wp_pool();
	WP *p1 = new_wp();
	WP *p2 = new_wp();
	WP *p3 = new_wp();
	free_wp(p2);
	mu_check(p3->next == p1);
	mu_check(p2->next->NO == 3);

	WP *p4 = new_wp();
	mu_check(p4 == p2);
	mu_check(p4->NO == 1);
	mu_check(p4->next == p3);

	WP *p5 = new_wp();
	free_wp(p3);
	free_wp(p1);

	mu_check(p5->next == p2);

	WP *p6 = new_wp();
	mu_check(p6 == p1);
	mu_check(p1->next == p5);
}

MU_TEST_SUITE(test_suite) {
	MU_RUN_TEST(test_watchpoint);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return MU_EXIT_CODE;
}
