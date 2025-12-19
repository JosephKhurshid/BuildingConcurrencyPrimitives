# BuildingConcurrencyPrimitives
Building different locks and barriers using atomic operations. The lock algorithms used include Test-And-Set (TAS), Test-Test-And-Set (TTAS), Ticket, and Mellor-Crummey and Scott (MCS). The barrier built includes the C++ provided barrier and the Sense Reveal barrier. 

The program prints out the length of time it takes to count up to the input number given the strategy used. You may only use one lock or one barrier but not both a lock and a barrier.


# Execution Examples

./counter [--name] [-t NUM_THREADS] [-n NUM_ITERATIONS] [--bar=<sense,pthread,senserel>] [--lock=<tas,ttas,ticket,mcs,pthread,peterson,tasrel,ttasrel,mcsrel,petersonrel>] [-o out.txt]`

