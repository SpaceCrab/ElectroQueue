/* STRUCT TO REPRESENT A POSITION IN THE SYSTEM */
struct Position
{
    int x;
    int y;
};

Position random_position();
Position nearest_charging_station();
void initialize_charging_stations();
void initialize_node();
void set_node(int lo, float bat_lev, float bat_con, int x_pos, int y_pos, int x_des, int y_des);
bool move(Position dest, int cs);
int calc_distance();
float calc_battery_consumption();
int calc_range();
float calc_priority();
void print_info();
void handle_move_destination();
void handle_move_charging_station();
void handle_connect_and_broadcast();
void handle_queuing();
void handle_charging();
void update_values();
void stateMachine();

/* DEFINE STATES */
enum State
{
    assign_new_destination,
    move_to_destination,
    move_to_nearest_charging_station,
    connect_and_broadcast,
    queuing,
    charging
};


/* CONSTANT VALUE THAT REPRESENTS THE MAXIMUM BATTERY-CONSUMPTION FOR A NODE*/
const float max_battery_consumption = MAX_BATTERY_LEVEL / MAX_DISTANCE;

/* A LIST OF ALL CHARGING STATIONS "CS" IN THE SYSTEM */
Position charging_stations[NR_OF_CS];

int distance;
int range;
int load;
float battery_level;
float battery_consumption;
int wait_count; // this will be changed to a variable that collects the time interval between each iteration (only when in state negotiate)
int broadcast_request;
float priority;
Position current_position;
Position destination;