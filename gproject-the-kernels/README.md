# This is a start code for CSC/CPE 159 operating system pragmatics.
 **Team member**:
- Mike Feschenko
- Viktor Tarasov
- Isaac Thawer

Implemented spinlock in phase5.1 tag for extra credit
### [04/22/2025] Phase 4 Feedback 
- **Strengths:**
  - The system compiles and runs
  - All of the system calls work
  - The tag is set correctly

- **Areas for Improvement:** 
  - It would be great to log all the system calls.
  - Sleep functin implementation is not correct. 


### [03/19/2025] Phase 3 Feedback 
- **Strengths:**
  - The system runs and the code compiles successfully.
  - Process scheduling works correctly.
  - The system correctly creates and deletes processes.  
  - All process operations are logged. Well done. 
  - The tag is set correctly. 
  - All functions are implemented.
  - The cursor is also implemented, great work. 

- **Areas for Improvement:**



### [03/03/2025] Phase 2 Feedback 
- **Strengths:**
  - System runs and code compiles successfully.
  - Cursor is implemented.
  - Timer and keyboard interrupts work.
  - Scrolling works. 
  - ESC correctly exits the OS.
  - TTY correctly switches when pressing ALT + (0 - 9).


- **Areas for Improvement:**
  - Input response is slow and does not always register key presses.
  - **Logging:**
    - No logs are generated when characters are printed.
    - CTRL + and CTRL - do not change the log level. I was only able to decrease the log level, then it seems to break. 
  - **Scrolling:**
    - Screen scrolling does not work but functions seem implemented.
    - HOME and END keys do not scroll the screen.
    - PAGE UP and PAGE DOWN keys do not scroll the screen
    - The bottom line text is covered by the interface. 



### [02/13/2025] Phase 1 Feedback 
- **Strengths:**
  - Implemented all functions.
  - No error found. 
  - keyboard implementation is greater!
  - kernel function is implemented.  
  - combo key works. 
  - Git comment is detailed and clean. 
 
- **Areas for Improvement:**
  - Trivial things: kernel log level increasement is unresponsible.  
