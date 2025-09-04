void error_argnum(int args_required, int args_provided, char* instruction_name, int line) {
	printf("Error: For instruction %s expected %d arguments but found %d at line %d\n", instruction_name, args_required, args_provided, line);	
	exit(1);
}
