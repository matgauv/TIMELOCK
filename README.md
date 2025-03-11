# TIMELOCK : team-03

## M2 Required Elements:
<table>
  <tr>
   <td><strong># / Task</strong>
   </td>
   <td><strong>Category</strong>
   </td>
   <td><strong>Implemented Where?</strong>
   </td>
   <td><strong>Notes</strong>
   </td>
  </tr>
  <tr>
   <td>[1] Decision Tree
   </td>
   <td>Improved Gameplay
   </td>
   <td><code>ai_system</code>
   </td>
   <td>Canon tower automatically aims and fires at the player if player is in its detection range
   </td>
  </tr>
  <tr>
   <td>[2] Sprite Animations
   </td>
   <td>Improved Gameplay
   </td>
   <td><code>animation_system</code>
   </td>
   <td>Sprite animations include run/walk and jump for the player, and exit door
   </td>
  </tr>
  <tr>
   <td>[3] Sprite and Asset Creation
   </td>
   <td>Improved Gameplay
   </td>
   <td><code>render_system</code>
   </td>
   <td>Ground/Ceiling tiles added, as well as other props (spikes, pipes, platforms, wooden crates)
   </td>
  </tr>
  <tr>
   <td>[4] Mesh Based Collision
   </td>
   <td>Improved Gameplay
   </td>
   <td><code>physics_system</code>
   </td>
   <td>Bolt mesh created for projectile-player collisions. Collisions are computed using SAT (separating axis theorem)
   </td>
  </tr>
  <tr>
   <td>[5] Gameplay Tutorial
   </td>
   <td>Improved Gameplay
   </td>
   <td><code>parsing_system and world_init</code>
   </td>
   <td>Levels are parsed from JSON. Text is rendered to the screen as sprites.
   </td>
  </tr>
  <tr>
   <td>[6] FPS counter
   </td>
   <td>Improved Gameplay
   </td>
   <td><code>world_system</code>
   </td>
   <td>Counter is displayed in the title of the window.
   </td>
  </tr>
  <tr>
   <td>[7]  2-minutes of unique gameplay
   </td>
   <td>Playability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>Two tutorial levels are implemented, each averaging about 2 minutes depending on the player. 
   </td>
  </tr>
  <tr>
   <td>[8]  Stable frame rate and minimal game lag.
   </td>
   <td>Stability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
Frame rate is especially good on hardware with dedicated graphics. Frame rate may fluctuate slightly on mobile hardware, but should stay above 60. We intentionally do not fix the frame rate at the moment to better observe how changes hurt performance. However, the physics system is stepped at a fixed rate. 
   </td>
  </tr>
  <tr>
   <td>[9] No crashes, glitches, or unpredictable behaviour.
   </td>
   <td>Stability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>[10] Test plan - a list of player or game actions and their expected outcomes.
   </td>
   <td>Software Engineering
   </td>
   <td>
   </td>
   <td>
<code>https://docs.google.com/document/d/1cRzYi9tH3jXyVs-ExeK_9EImm4sDYJd5uW0GMP75GQw/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[11] Bug List
   </td>
   <td>Reporting
   </td>
   <td>
   </td>
   <td>
<code>https://docs.google.com/spreadsheets/d/14reigVHCiUrnIVMnTQdUnz6nX_JFBuTTT_pZXGZ06WA/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[12] Video
   </td>
   <td>Reporting
   </td>
   <td>
   </td>
   <td>
<code>https://www.youtube.com/watch?v=p3mYLYqT8v8</code>
   </td>
  </tr>
</table>



# M2 Creative Elements

** these are the creative elements we wish to have graded for this milestone

<table>
  <tr>
   <td><strong>Element</strong>
   </td>
   <td><strong>Category</strong>
   </td>
   <td><strong>Level</strong>
   </td>
   <td><strong>Implemented Where? </strong>
   </td>
   <td><strong>Description</strong>
   </td>
  </tr>
  <tr>
   <td>[1] Simple rendering Effects
   </td>
   <td>Graphics
   </td>
   <td>Basic
   </td>
   <td><code>rendering_system</code>
   </td>
   <td>We use fragment shaders to put an effect on the screen when the player is using the time accelerate and decelerate powers.
   </td>
  </tr>
  <tr>
   <td>[24] Basic integrated assets
   </td>
   <td>Quality & User Experience (UX)
   </td>
   <td>Basic
   </td>
   <td><code>render_system</code>
   </td>
   <td>Basic assets for the game have been included
   </td>
  </tr>
</table>



## Other Notes:
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
