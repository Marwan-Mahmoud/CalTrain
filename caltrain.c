#include <pthread.h>
#include "caltrain.h"

void station_init(struct station *station)
{
	station->waitingPassengers = 0;
	station->availableSeats = 0;
	station->boardingPassengers = 0;
	pthread_mutex_init(&station->mutex, NULL);
	pthread_cond_init(&station->passengersBoardedCond, NULL);
	pthread_cond_init(&station->seatAvailableCond, NULL);
}

void station_load_train(struct station *station, int count)
{
	pthread_mutex_lock(&station->mutex);
	station->availableSeats = count;
	// Wait until all passengers have boarded or train is full.
	if (station->waitingPassengers > 0 && station->availableSeats > 0)
	{
		pthread_cond_broadcast(&station->seatAvailableCond);
		pthread_cond_wait(&station->passengersBoardedCond, &station->mutex);
	}
	station->availableSeats = 0; // Train left
	pthread_mutex_unlock(&station->mutex);
}

void station_wait_for_train(struct station *station)
{
	pthread_mutex_lock(&station->mutex);
	station->waitingPassengers++;
	// Wait until there is a train and a seat is available.
	while (station->availableSeats == 0)
	{
		pthread_cond_wait(&station->seatAvailableCond, &station->mutex);
	}
	station->waitingPassengers--;
	station->availableSeats--;
	station->boardingPassengers++;
	pthread_mutex_unlock(&station->mutex);
}

void station_on_board(struct station *station)
{
	pthread_mutex_lock(&station->mutex);
	// A passenger has boarded.
	station->boardingPassengers--;
	// All passengers are in their seats and either the train is full or there are no waiting passengers.
	if (station->boardingPassengers == 0 && (station->availableSeats == 0 || station->waitingPassengers == 0))
	{
		pthread_cond_signal(&station->passengersBoardedCond);
	}
	pthread_mutex_unlock(&station->mutex);
}
