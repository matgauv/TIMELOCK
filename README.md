# TIMELOCK : team-03

## M1 Required Elements:
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
   <td>[1] Textured Geometry
   </td>
   <td>Rendering
   </td>
   <td><code>rendering_system</code>
   </td>
   <td>The character and background layers are sprites which have been textured (using the template provided by A1) 
   </td>
  </tr>
  <tr>
   <td>[2] Basic 2D transformations
   </td>
   <td>Rendering
   </td>
   <td><code>rendering_system</code>
   </td>
   <td>All entities contain a motion component which specifies scale, angle, and position factors. 
<p>
For M1, scaling factors (used for background images) and position factors (for moving platforms and character) are passed to transformation matrices to correctly scale and translate entities. 
   </td>
  </tr>
  <tr>
   <td>[3] Key-frame / state interpolation
   </td>
   <td>Rendering
   </td>
   <td><code>physics_system </code>
   </td>
   <td>The time control systems use linear interpolation to smoothly transition between accelerated and decelerated states.
<p>
Based on the required transition between time control states, we interpolate the <code>velocityModifier</code> so that it smoothly transitions over a short period of time.
   </td>
  </tr>
  <tr>
   <td>[4] Keyboard / mouse control
   </td>
   <td>Gameplay
   </td>
   <td><code>world_system</code>
   </td>
   <td>The player can move left/right using the <strong>arrow keys</strong> and jump with the <strong>up arrow</strong>.
<p>
The player can activate time accelerate with <strong>Q</strong>, and time decelerate with <strong>W</strong>.
   </td>
  </tr>
  <tr>
   <td>[5] Random/coded action
   </td>
   <td>Gameplay
   </td>
   <td><code>physics_system</code>
   </td>
   <td>There are moving platforms that follow a predefined path.
   </td>
  </tr>
  <tr>
   <td>[6] Well-defined game-space boundaries
   </td>
   <td>Gameplay
   </td>
   <td><code>world_init</code>
   </td>
   <td>Rigid ‘platforms’ are used to create a box around the world space. This prevents the player from leaving the defined play space, while letting us easily scale levels to any dimension we need. If players touch this boundary, the level is reset.
   </td>
  </tr>
  <tr>
   <td>[7]  Simple collision detection & resolution (e.g. between square sprites)
   </td>
   <td>Gameplay
   </td>
   <td><code>physics_system </code>
   </td>
   <td>The physics system handles basic collisions between physics objects with square bounding boxes. Collisions are detected with AABB and handled based on the types of objects colliding and the side of the collision. 
   </td>
  </tr>
  <tr>
   <td>[8]  Stable frame rate and minimal game lag.
   </td>
   <td>Stability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>Framerate is decent, though some systems (physics + rendering) might be a little inefficient.
   </td>
  </tr>
  <tr>
   <td>[9] No crashes, glitches, or unpredictable behaviour.
   </td>
   <td>Stability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>Game does not crash under normal gameplay. Systems are working as expected.
<ul>

<li>Physics edge cases can feel a little weird, but this is not unexpected, rather just a result of the physics implementation.</li>
</ul>
   </td>
  </tr>
  <tr>
   <td>[10] Test plan - a list of player or game actions and their excepted outcomes.
   </td>
   <td>Software Engineering
   </td>

   </td>
   <td>
   <td><code>doc/test-plan.docx</code>, or <code>https://docs.google.com/document/d/13LJrqoD_sT1gskjJNIik9PtfUHwcw2Ep8w6D4uNXUX4/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[11] Bug List
   </td>
   <td>Reporting
   </td>
   </td>
   <td>
    <td><code>doc/bug-report.xlsx</code>, or <code>https://docs.google.com/spreadsheets/d/14reigVHCiUrnIVMnTQdUnz6nX_JFBuTTT_pZXGZ06WA/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[12] Video
   </td>
   <td>Reporting
   </td>
   </td>
   <td>
      <td><code>https://youtu.be/pkuMfbTBf-4</code>
   </td>
  </tr>
</table>



## M1 Creative Elements

** we have included all of the features we have implemented that meet the criteria specified on suggested game features list.


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
   <td>[2] Parallax scrolling backgrounds
   </td>
   <td>Graphics
   </td>
   <td>Basic
   </td>
   <td><code>rendering_system</code>
<p>
<code>camera_system</code>
   </td>
   <td>There are 3 layers to the game, background, midground, and foreground. When the camera moves, these layers move at different speeds giving a parallax effect.
   </td>
  </tr>
  <tr>
   <td>[8] Basic physics
   </td>
   <td>Physics & Simulation
   </td>
   <td>Basic
   </td>
   <td><code>physics_system </code>
   </td>
   <td>Basic physics are implemented, allowing a player to interact with moving platforms, and push some objects around.
<p>
Basic phenomena are approximated, such as gravity, friction, and inertia.
   </td>
  </tr>
  <tr>
   <td>[21] Camera Controls
   </td>
   <td>User Interface (UI) & Input/Output (IO)
   </td>
   <td>Basic
   </td>
   <td><code>camera_system</code>
   </td>
   <td>The camera follows the player as they navigate the level. 
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
   <td>Some basic assets for the game have been included!
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
