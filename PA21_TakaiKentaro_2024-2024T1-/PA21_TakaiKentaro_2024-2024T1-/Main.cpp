# include <Siv3D.hpp>

/// @brief ブロックのサイズ
constexpr Size BRICK_SIZE{ 50, 25 };

/// @brief ボールの速さ
//constexpr double BALL_SPEED = 500.0;

/// @brief ブロックの数　縦
constexpr int Y_COUNT = 10;

/// @brief ブロックの数　横
constexpr int X_COUNT = 20;

/// @brief 合計ブロック数
constexpr int MAX = Y_COUNT * X_COUNT;

constexpr enum GameState {
	Title,
	InGame,
	Result,
};

void Main()
{
#pragma region Game
	GameState game = Title;

#pragma endregion

#pragma region Start
	const Font startFont{ 50, Typeface::Black };
#pragma endregion

#pragma region Result
	const Font resultFont{ 50,Typeface::Black };
	constexpr Vec2 resultPos{ 400,50 };
	const Font restartFont{ 40,Typeface::Black };
	constexpr Vec2 restartPos{ 400,500 };
#pragma endregion

#pragma region Score
	// フォントを作成
	const Font scoreFont{ 25, Typeface::Black };
	constexpr Vec2 scorePos{ 650,20 };
	int score = 0;
#pragma endregion

#pragma region Timer
	// フォントを作成
	const Font timerFont{ 25, Typeface::Black };
	constexpr Vec2 timerPos{ 150,20 };
	int timer = 60;
	Stopwatch stopwatch{ StartImmediately::Yes };
#pragma endregion

#pragma region Ball
	/// @brief ボールの速度
	float ballSpeed = 500.0;
	Vec2 ballVelocity{ 0, -500.0 };

	/// @brief ボール
	Circle ball{ 400, 400, 10 };
#pragma endregion

#pragma region Gun
	const Vec2 muzzle;

	Circle bullet{ muzzle.x,muzzle.y,5 };
	double bulletSpeed = 300.0;
	Vec2 bulletVelocity{ 0,-300.0 };
	Array<Vec2> bullets;
#pragma endregion

#pragma region Bricks
	/// @brief ブロック
	Rect bricks[MAX];

	// ブロックを初期化
	for (int y = 0; y < Y_COUNT; ++y) {
		for (int x = 0; x < X_COUNT; ++x) {
			int index = y * X_COUNT + x;
			bricks[index] = Rect{
				x * BRICK_SIZE.x,
				60 + y * BRICK_SIZE.y,
				BRICK_SIZE
			};
		}
	}
#pragma endregion

	while (System::Update())
	{
		//==============================
		// 更新
		//==============================
		switch (game)
		{
		case Title://タイトル画面
		{
			// 初期化=======================
			ballSpeed = 500.0;
			ballVelocity = { 0, -500.0 };
			ball = { 400,400,10 };

			for (int y = 0; y < Y_COUNT; ++y) {
				for (int x = 0; x < X_COUNT; ++x) {
					int index = y * X_COUNT + x;
					bricks[index] = Rect{
						x * BRICK_SIZE.x,
						60 + y * BRICK_SIZE.y,
						BRICK_SIZE
					};
				}
			}

			timer = 60;
			score = 0;
			//==============================

			for (int i = 0; i < MAX; ++i) {
				bricks[i].stretched(-1).draw(HSV{ bricks[i].y - 40 });
			}

			// ボール描画
			ball.draw();

			// パドル描画
			const Rect paddle{ Arg::center(Cursor::Pos().x, 500), 100, 10 };
			paddle.rounded(3).draw();
			scoreFont(U"Score ", score).draw(Arg::center = scorePos);
			timerFont(U"Time ", timer).draw(Arg::center = timerPos);
			startFont(U"Click Start").drawAt(Scene::Center());

			if (MouseL.down()) { game = InGame; }
		}
		break;
		case InGame://インゲーム
		{
			//スコア
			// 左上位置 (20, 20) からテキストを描く
			scoreFont(U"Score ", score).draw(Arg::center = scorePos);

			//タイマー
			timerFont(U"Time ", timer).draw(Arg::center = timerPos);
			if (stopwatch.sF() >= 1.0) {
				timer--;
				ballSpeed += 10;
				stopwatch.restart();
				if (timer <= 0) {
					game = Result;
				}
			}

			// パドル
			const Rect paddle{ Arg::center(Cursor::Pos().x, 500), 100, 10 };

			// ボール移動
			ball.moveBy(ballVelocity * Scene::DeltaTime());

			// バレット移動
			bullet.moveBy(bulletVelocity * Scene::DeltaTime());

			//==============================
			// コリジョン
			//==============================
			// ブロックとの衝突を検知
			for (int i = 0; i < MAX; ++i) {
				// 参照で保持
				Rect& refBrick = bricks[i];

				// 衝突を検知
				if (refBrick.intersects(ball)) {
					// ブロックの上辺、または底辺と交差
					if (refBrick.bottom().intersects(ball) || refBrick.top().intersects(ball))
					{
						ballVelocity.y *= -1;
					}
					else // ブロックの左辺または右辺と交差
					{
						ballVelocity.x *= -1;
					}

					// あたったブロックは画面外に出す
					refBrick.y -= 600;

					//スコア加算
					score++;

					// 同一フレームでは複数のブロック衝突を検知しない
					break;
				}
				if (refBrick.intersects(bullet)) {
					// あたったブロックは画面外に出す
					refBrick.y -= 600;
					//スコア加算
					score++;
					//バレットの削除
					bullets.clear();
				}
			}

			// 天井との衝突を検知
			if ((ball.y < 0) && (ballVelocity.y < 0))
			{
				ballVelocity.y *= -1;
			}

			// 床との衝突を検知
			if ((ball.y > Scene::Height()) && (ballVelocity.y > 0))
			{
				game = Result;
				//ballVelocity.y *= -1;
			}

			// 壁との衝突を検知
			if (((ball.x < 0) && (ballVelocity.x < 0))
				|| ((Scene::Width() < ball.x) && (0 < ballVelocity.x)))
			{
				ballVelocity.x *= -1;
			}

			// パドルとの衝突を検知
			if ((0 < ballVelocity.y) && paddle.intersects(ball))
			{
				ballVelocity = Vec2{
					(ball.x - paddle.center().x) * 10,
					-ballVelocity.y
				}.setLength(ballSpeed);
			}

			//==============================
			// 描画
			//==============================
			// ブロック描画
			for (int i = 0; i < MAX; ++i) {
				bricks[i].stretched(-1).draw(HSV{ bricks[i].y - 40 });
			}

			// ボール描画
			ball.draw();

			// パドル描画
			paddle.rounded(3).draw();
		}
		break;
		case Result://リザルト
		{
			resultFont(U"RESULT").draw(Arg::center = resultPos);
			scoreFont(U"Score ：", score).drawAt(Scene::Center());
			restartFont(U"Click Title").draw(Arg::center = restartPos);
			if (MouseL.down()) { game = Title; }
		}
		break;
		default: {

		}
			   break;
		}
	}
}
