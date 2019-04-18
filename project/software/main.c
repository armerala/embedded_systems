
int main()
{
    /*the big init*/
	if(init_input() != 0)
        exit(1);
    //init scene
    //init physics

    //the game loop
    while(1)
    {
        update_inputs();
        //run scene update
        //run physics update
    }

    return 0;
}
