#include <iostream>
#include "godiceapi.h"

using namespace std;

void test_stables() {
	godiceapi_callbacks_t callbacks = {
		.on_dice_roll = [](void *userdata, int dice_id, int number) {
			cout << number << endl;
		}
	};
	{
		uint8_t packet[] = {'S', 0, 0, (uint8_t)-64};
		incoming_packet(&callbacks, nullptr, 0, packet, sizeof(packet));
	}
	{
		uint8_t packet[] = {'F', 'S', 128, 128, 128};
		incoming_packet(&callbacks, nullptr, 0, packet, sizeof(packet));
	}
}

int main() {
	test_stables();
	return 0;
}
