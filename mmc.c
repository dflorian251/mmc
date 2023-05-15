/*  External definitions for single-server queueing system.  */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "rand.h"		/*  Header file for random-number generator.  */

#define Q_LIMIT 100		/*  Limit on queue length.  */
#define BUSY 1			/*  Mnemonic for server's being busy,  */
#define IDLE 0			/*  and idle.  */
#define C 2             // the number of servers

int       next_event_type, num_custs_delayed, num_delays_required,
            num_events, num_in_q, server_status[C], server_used;
float    area_num_in_q, area_server_status, mean_interarrival,
            mean_service, time, time_arrival[Q_LIMIT + 1],
            time_last_event, time_next_event[C+2], total_of_delays;
FILE   *infile, *outfile;

void   initialize(void);
void   timing(void);
void   arrive(void);
void   depart(void);
void   report(void);
void   update_time_avg_stats(void);
float  expon(float mean);
int check_c_servers(void);
int pick_a_server(int status);


/*  Main function  */
int main(void)
{
	/*  Open input and output files.  */
	infile    = fopen("mmc.in","r"); //isnt used
	outfile  = fopen("mmc.out","w");

	/*  specify the number of events for the timing function.  */
	num_events = C + 1;

	/*  Read input parameters.  */
	// fscanf(infile, "%f  %f  %d",  &mean_interarrival,  &mean_service,
	// 	&num_delays_required);
    mean_interarrival = 0.5555555556;
    mean_service = 0.5;
    num_delays_required = 1000000;

	/*  Write report heading and input parameters.  */
	fprintf(outfile,  "C-server queueing system\n\n");
	fprintf(outfile,  "Mean interarrival time%11.3f  minutes\n\n",  mean_interarrival);
	fprintf(outfile,  "Mean service time%16.3f  minutes\n\n",  mean_service);
	fprintf(outfile,  "Number of customers%14d\n\n",  num_delays_required);
    fprintf(outfile,  "Number of servers%14d\n\n",  C);

	/*  Initialize the simulation.  */
	initialize( );

	/*  Run the simulation while more delays are still needed.  */
	while (num_custs_delayed < num_delays_required)  {

		/*  Determine the next event.  */
		timing( );

		/*  Update time_average statistical accumulators.  */
		update_time_avg_stats( );
        printf("%d\n",next_event_type );
        if(next_event_type == 1){
            arrive();
        }else{
            depart();
        }


	}

	/*  Invoke the report generator and end the simulation.  */
	report( );
	fclose(infile);
	fclose(outfile);
	return 0;
}

/*  Initialization function.  */
void initialize(void)
{
	/*  Initialize the simulation clock.  */

	time = 0.0;

	/*  Initialize the state variables.  */

    for(int i=0;i<C;i++){
        server_status[i]=  IDLE;
    }
	num_in_q =  0;
	time_last_event =  0.0;

	/*  Initialize the statistical counters.  */

	num_custs_delayed   =  0;
	total_of_delays           =  0.0;
	area_num_in_q           =  0.0;
	area_server_status     =  0.0;

	/*   Initialize event list. Since no customers are present, the departure (service
                 completion) event is eliminated from consideration.      */

	time_next_event[1] = time + expon(mean_interarrival);
    for(int i=2;i<=num_events;i++){
	   time_next_event[i] = 1.0e+30;
    }
}

/*  Timing function.  */
void timing(void)
{
	int   i;
	float min_time_next_event = 1.0e+29;
	next_event_type = 0;

	/*  Determine the event type of the next event to occur.  */

	for  (i = 1;  i <= num_events;  ++i)   {
		if  (time_next_event[i] < min_time_next_event)    {
			min_time_next_event = time_next_event[i];
			next_event_type = i;
		}
	}
	/*  Check to see whether the event list is empty.  */

	if  (next_event_type == 0)     {

		/*  The event list is empty, so stop the simulation.  */

		fprintf(outfile, "\nEvent list empty at time %f", time);
		exit(1);
	}

	/*  The event list is not empty, so advance the simulation clock.  */

	time = min_time_next_event;


}

/*  Arrival event function.  */
void arrive(void)
{
	float delay;

	/*  Schedule next arrival.  */

	time_next_event[1] = time + expon(mean_interarrival);

	/*  Check to see whether server is busy.  */

	if  (check_c_servers() == BUSY)      {

		/*  Server is busy, so increment number of customers in queue.  */

		++num_in_q;

		/*  Check to see whether an overflow condition exists.  */

		if  (num_in_q > Q_LIMIT)     {

			/*  The queue has overflowed, so stop the simulation.  */

			fprintf(outfile, "\nOverflow of the array  time_arrival  at");
			fprintf(outfile, " time  %f",  time);
			exit(2);
		}

		/*  There is still room in the queue, so store the time of arrival of the
                           arriving customer at the (new) end of time_arrival.  */

		time_arrival[num_in_q] = time;
	}

	else    {

		/*  Server is idle, so arriving customer has a delay of zero. (The following
                       two statements are for program clarity and do not affect the results.)  */
		delay            =  0.0;
		total_of_delays += delay;

		/* Increment the number of customers delayed, and make server busy. */

		++num_custs_delayed;
        server_used = pick_a_server(IDLE);
		server_status[server_used] = BUSY;

		/*  Schedule a departure (service completion).  */
		time_next_event[server_used + 2] = time + expon(mean_service);

    }
}

/*  Departure event function.  */
void  depart(void)
{
	int  i;
	float delay;

	/*  Check to see whether the queue is empty.  */

	if  (num_in_q == 0)     {

		/*  The queue is empty, so make the server idle and eliminate the
                           departure (service comletion)  event from consideration.  */


        server_status[next_event_type - 2] = IDLE;
        time_next_event[next_event_type]  =  1.0e+30;

		// server_status     = IDLE;
		// time_next_event[2]  =  1.0e+30;
	}

	else   {

		/*  The queue is nonempty, so decrement the number of customers
                           in queue.  */

		--num_in_q;

		/*  Compute the delay of the customer who is beginning service and
                           update the total delay accumulator.  */

		delay     =     time - time_arrival[1];
		total_of_delays += delay;

		/* Increment the number of customers delayed, and schedule departure.*/

		++num_custs_delayed;
		time_next_event[next_event_type] = time + expon(mean_service);

		/*  Move each customer in queue (if any) up one place.  */

		for  (i = 1;  i <= num_in_q;  ++i)
			time_arrival[i] = time_arrival[i + 1];
	}
}

/*  Report generator function.  */
void  report(void)
{
	/*  Compute and write estimates of desired measures of performance.  */
    area_server_status = area_server_status / C;
	fprintf(outfile, "\n\nAverage delay in queue%11.7f minutes\n\n",
		total_of_delays / num_custs_delayed);
	fprintf(outfile, "Average number in queue%10.7f\n\n",
		area_num_in_q / time);
	fprintf(outfile, "Server utilization%15.7f\n\n",
		area_server_status / time);
	fprintf(outfile, "Time simulation ended%12.3f", time);
}

/*  Update area accumulators for time-average statistics.  */
void  update_time_avg_stats(void)
{
	float time_since_last_event;

	/*  Compute time since last event, and update last-event-time marker.  */

	time_since_last_event = time - time_last_event;
	time_last_event         = time;

	/*  Update area under number-in-queue function.  */

	area_num_in_q  += num_in_q * time_since_last_event;

	/*  Update area under server-busy indicator function.  */
    for(int i=0;i<C;i++){
        area_server_status += server_status[i] * time_since_last_event;
    }
}

/*  Exponential variate generation function.  */
float  expon(float mean)
{
	float  u;

	/*  Generate a  U(0,1)  random variate.  */

	u = myrand(1);

	/*  Return an exponential random variate with mean "mean".  */

	return   -mean * log(u);
}

/* Checks the total status of the C servers
 if all the servers are IDLE, returns IDLE
 if all the servers are BUSY, returns BUSY
if some of the servers are IDLE and others are BUSY, returns 2
*/
int check_c_servers(void){
    int sum_idle, sum_busy = 0;
    for(int i=0; i<C; i++){
        if (server_status[i] == IDLE){
            sum_idle++;
        }else{
            sum_busy++;
        }
    }
    if(sum_idle == C)
        return IDLE;

    if(sum_busy == C)
        return BUSY;
    return 2;
}

/* Returns the index of the server based on the status */
int pick_a_server(int status){
    for(int i=0; i<C; i++){
        if(server_status[i]==status)
            return i;
    }
}
