# TIMELOCK
A 2D platformer where players must chase their ringing alarm clock through a clockwork castle by successfully navigating through a series of rooms which are themed on different time based aesthetics like pendulums swinging or clock gears turning. Use time-based deceleration to navigate the castle's many swift obstacles and snooze your alarm clock to return to a peaceful slumber!

*Created by: Yixian Cheng, Dieter Frehlich, Matias Gauvin, Julia Rees, and Kevin Zhou*

https://github.com/user-attachments/assets/47f1e181-79a9-4ca4-952d-c0d5f5cbccb7

## Tech stack:
- Core game logic - **C++**
- Graphics engine - **OpenGL**
- Level construction - **LDtk w/ JSON parsing**


## Other Notes:
- DEBUG CONTROLS: `N` will enter `no-clip` mode, `M` will enter fly mode (use arrow keys to fly). Use for exploring the level when needed, make sure to disable both before playing again!
- Background music source: https://www.youtube.com/watch?v=gfnQIaFBKIE
- Slowdown & speedup sound effect source: https://www.youtube.com/watch?v=z_epWpdsHJc
- How to run on linux/mac (and compile with debug symbols):
  - Navigate to `ECS_DRAFT/build`.
  - Run `cmake ..` (or `cmake -DCMAKE_BUILD_TYPE=Debug ..` for debug symbols)
  - Run `make` to compile changes
  - Run `./TIMELOCK`

- How to build on Windows Visual Studio:
  - Open folder `ECS_DRAFT`, wait for CMakeList.txt file to be loaded
  - Can also manually configure CMake cache by going to `Navigation bar` -> `Project` -> `Configure Cache`
  - Update CMake settings according to need; we suggest build and run release version of the game
    - Windows user can go to `/ECS_DRAFT/CMakeSettings.json` and add build modes, such as `x64-Release`
  - Build `TIMELOCK.exe`
  - Run `TIMELOCK.exe`
 
- How to use co author script:
    - make sure it is executable (`chmod +x ./coAuthorCommit.sh`)
    - Add all the files to git (`git add -p`)
    - Run the commit script to make the commit `./coAuthorCommit.sh "NAME" "EMAIL" "COMMIT MESSAGE"`
    - Push (`git push`)
