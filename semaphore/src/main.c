/*Increments shared ccounter for ping pong
 * Author: Stuti*/


#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

/*Shared counter and mutex*/
static int shared_counter = 0;
static struct k_mutex counter_mutex;

K_SEM_DEFINE(sem_ping, 1, 1);
K_SEM_DEFINE(sem_pong, 0, 1);

/*Ping thread */
void ping_thread(void *a, void *b, void *c)
{
	while(1){
		k_sem_take(&sem_ping, K_FOREVER);

		k_mutex_lock(&counter_mutex, K_FOREVER);
		shared_counter++;
		printk("Ping counter=%d\n", shared_counter);
		k_mutex_unlock(&counter_mutex);

		k_sleep(K_MSEC(500));
		k_sem_give(&sem_pong);
	}
}

/*Pong thread */
void pong_thread(void *a, void *b, void *c){
	while(1){
		k_sem_take(&sem_pong, K_FOREVER);
		
		k_mutex_lock(&counter_mutex, K_FOREVER);
		shared_counter++;
		printk("Pong counter=%d\n", shared_counter);
		k_mutex_unlock(&counter_mutex);

		k_sleep(K_MSEC(500));
		k_sem_give(&sem_ping);
	}
}

/*Thread creation*/
K_THREAD_DEFINE(ping_id, 1024, ping_thread, NULL, NULL, NULL, 1, 0, 0);
K_THREAD_DEFINE(pong_id, 1024, pong_thread, NULL, NULL, NULL, 1, 0, 0);

int main(void){
	k_mutex_init(&counter_mutex);
	printk("Ping-Pong with shared counter using mutex\n");
	return 0;
}
