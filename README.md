In this project, I developed a functional irrigation system for a lawn.
The system includes a valve that is connected to a water tap and controls the irrigation process.

It is integrated with a Firebase Realtime Database. When the program starts, it first synchronizes with the database and updates the two irrigation time slots, the duration of irrigation, and whether irrigation is enabled or not.

Additionally, a callback function allows real-time updates to be applied to the system whenever parameters are changed.

At the specified time slots, and if irrigation is enabled, the system will activate irrigation for the defined duration.
