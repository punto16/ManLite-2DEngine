
using ManLiteScripting;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PongGameManager : MonoBehaviour
{
    //create variables here
    public IGameObject player1_go;
    public IGameObject player2_go;

    public float players_speed = 1.0f;

    private int player1_score = 0;
    private int player2_score = 0;

    private Collider2D player1_collider;
    private Collider2D player2_collider;

    private Transform player1_transform;
    private Transform player2_transform;

    private Vec2f player1_start_position;
    private Vec2f player2_start_position;

    private bool play_mode = false;
    private float wait_time = 2.0f;
    private float wait_timer = 0.0f;

    public IGameObject canvas_go;
    private Canvas canvas;

    public IGameObject ball_go;
    private Transform ball_transform;
    private PongBall ball_script;

    //will be called once
    public override void Start()
    {
        player1_score = 0;
        player2_score = 0;

        player1_collider = new Collider2D(player1_go);
        player2_collider = new Collider2D(player2_go);

        player1_transform = new Transform(player1_go);
        player2_transform = new Transform(player2_go);

        player1_start_position = player1_transform.GetWorldPosition();
        player2_start_position = player2_transform.GetWorldPosition();

        canvas = new Canvas(canvas_go);

        ball_transform = new Transform(ball_go);
        ball_script = MonoBehaviour.GetComponent<PongBall>(ball_go);
    }

    //it is called every frame
    public override void Update()
    {
        float dt = ML_System.GetDT();

        if (play_mode)
        {
            //input handling
            Vec2f player_1_speed = new Vec2f();
            player_1_speed.X = 0.0f;
            player_1_speed.Y = 0.0f;

            if (Input.GetKeyboardKey(KeyboardKey.KEY_W) == KeyState.KEY_REPEAT)
            {
                player_1_speed.Y += players_speed;
            }
            if (Input.GetKeyboardKey(KeyboardKey.KEY_S) == KeyState.KEY_REPEAT)
            {
                player_1_speed.Y -= players_speed;
            }
            player1_collider.SetSpeed(player_1_speed);


            Vec2f player_2_speed = new Vec2f();
            player_2_speed.X = 0.0f;
            player_2_speed.Y = 0.0f;

            if (Input.GetKeyboardKey(KeyboardKey.KEY_UP) == KeyState.KEY_REPEAT)
            {
                player_2_speed.Y += players_speed;
            }
            if (Input.GetKeyboardKey(KeyboardKey.KEY_DOWN) == KeyState.KEY_REPEAT)
            {
                player_2_speed.Y -= players_speed;
            }

            player2_collider.SetSpeed(player_2_speed);

        }
        else
        {
            Vec2f stop_moving = new Vec2f();
            player1_collider.SetSpeed(stop_moving);
            player2_collider.SetSpeed(stop_moving);


            wait_timer += dt;
            if (wait_timer >= wait_time)
            {
                StartGame();
                wait_timer = 0.0f;
            }
        }
    }

    public void StartGame()
    {
        play_mode = true;

        if (canvas.IsItemUIEnabled("Title1"))       canvas.SetItemUIEnabled("Title1", false);
        if (canvas.IsItemUIEnabled("Subtitle1"))    canvas.SetItemUIEnabled("Subtitle1", false);

        if (player1_score >= 10 || player2_score >= 10)
        {
            player1_score = 0;
            player2_score = 0;

            string player1_score_string = "Player 1: " + player1_score.ToString();
            canvas.SetText("Player1", player1_score_string);
            string player2_score_string = "Player 2: " + player2_score.ToString();
            canvas.SetText("Player2", player2_score_string);
        }

        player1_transform.SetWorldPosition(player1_start_position);
        player2_transform.SetWorldPosition(player2_start_position);

        ball_transform.SetWorldPosition(new Vec2f(0.0f, 0.0f));
        ball_script.StartBall();
    }

    //  true -> player 1 scored  //  false -> player 2 scored
    public void PlayerScored(bool who_scored, bool draw)
    {
        play_mode = false;

        if (draw)
        {
            ball_transform.SetWorldPosition(new Vec2f(0.0f, -10.0f));
            return;
        }

        if (who_scored) player1_score += 1;
        else player2_score += 1;

        if (player1_score >= 10)
        {
            canvas.SetItemUIEnabled("Title1", true);
            canvas.SetItemUIEnabled("Subtitle1", true);
            canvas.SetText("Title1", "GAME OVER");
            canvas.SetText("Subtitle1", "PLAYER 1 WON THE MATCH");
            wait_timer = -3.0f;
        }
        else if (player2_score >= 10)
        {
            canvas.SetItemUIEnabled("Title1", true);
            canvas.SetItemUIEnabled("Subtitle1", true);
            canvas.SetText("Title1", "GAME OVER");
            canvas.SetText("Subtitle1", "PLAYER 2 WON THE MATCH");
            wait_timer = -3.0f;
        }

        string player1_score_string = "Player 1: " + player1_score.ToString();
        canvas.SetText("Player1", player1_score_string);
        string player2_score_string = "Player 2: " + player2_score.ToString();
        canvas.SetText("Player2", player2_score_string);

        ball_transform.SetWorldPosition(new Vec2f(0.0f, -10.0f));

    }

    //functions for colliders
    public override void OnTriggerCollision(IGameObject other)
    {
    }
    public override void OnExitCollision(IGameObject other)
    {
    }

    public override void OnTriggerSensor(IGameObject other)
    {
    }
    public override void OnExitSensor(IGameObject other)
    {
    }
}