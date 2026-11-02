#include <LiquidCrystal_I2C.h>

// LCD 주소가 다르면 0x27을 실제 주소로 변경
LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte POT_PIN = A0;

// 달리는 사람: 다리 모양이 다른 두 프레임
byte runnerA[8] = {
  B00100,
  B01110,
  B00100,
  B01101,
  B10110,
  B00100,
  B01010,
  B10001
};

byte runnerB[8] = {
  B00100,
  B01110,
  B00100,
  B10110,
  B01101,
  B00100,
  B01010,
  B01010
};

// 막대의 부분 채움 문자: 왼쪽부터 1~4열
// 사용자 정의 문자 번호 2~5에 저장
byte barParts[4][8] = {
  {
    B10000, B10000, B10000, B10000,
    B10000, B10000, B10000, B10000
  },
  {
    B11000, B11000, B11000, B11000,
    B11000, B11000, B11000, B11000
  },
  {
    B11100, B11100, B11100, B11100,
    B11100, B11100, B11100, B11100
  },
  {
    B11110, B11110, B11110, B11110,
    B11110, B11110, B11110, B11110
  }
};

// 완전히 채워진 막대: 사용자 정의 문자 번호 6
byte fullBar[8] = {
  B11111, B11111, B11111, B11111,
  B11111, B11111, B11111, B11111
};

byte runnerColumn = 0;
byte runnerFrame = 0;

unsigned long FRAME_INTERVAL = 300;
unsigned long MOVE_INTERVAL  = 400;
const unsigned long BAR_INTERVAL   = 50;

unsigned long lastFrameTime = 0;
unsigned long lastMoveTime  = 0;
unsigned long lastBarTime   = 0;

int previousBarPixels = -1;

// 둘째 줄 막대 갱신
void updateBar() {
  int potValue = analogRead(POT_PIN);

  // 가변저항 값이 클수록 이동 간격이 짧아져 빨라짐
  // 최솟값: 600ms마다 이동
  // 최댓값: 80ms마다 이동
  MOVE_INTERVAL = map(potValue, 0, 1023, 600, 80);

  // 이동 속도에 맞춰 프레임 속도도 조절
  FRAME_INTERVAL = MOVE_INTERVAL / 2;

  // 16칸 × 한 칸당 5열 = 총 80단계
  int barPixels = map(potValue, 0, 1023, 0, 80);

  // 표시 길이가 같으면 다시 쓰지 않음
  if (barPixels == previousBarPixels) {
    return;
  }

  previousBarPixels = barPixels;
  lcd.setCursor(0, 1);

  for (byte column = 0; column < 16; column++) {
    int fill = barPixels - column * 5;

    if (fill <= 0) {
      lcd.print(' ');
    } else if (fill >= 5) {
      lcd.write(byte(6));
    } else {
      // fill 1, 2, 3, 4 → 문자 번호 2, 3, 4, 5
      lcd.write(byte(fill + 1));
    }
  }
}

void setup() {
  pinMode(POT_PIN, INPUT);

  lcd.init();
  lcd.backlight();

  // 사용자 정의 문자 등록
  lcd.createChar(0, runnerA);
  lcd.createChar(1, runnerB);

  for (byte i = 0; i < 4; i++) {
    lcd.createChar(i + 2, barParts[i]);
  }

  lcd.createChar(6, fullBar);

  lcd.clear();

  // 처음에는 왼쪽 끝에 사람 표시
  lcd.setCursor(runnerColumn, 0);
  lcd.write(runnerFrame);

  updateBar();
}

void loop() {
  unsigned long now = millis();
  bool runnerChanged = false;

  // 다리 모양 변경
  if (now - lastFrameTime >= FRAME_INTERVAL) {
    lastFrameTime = now;

    runnerFrame = 1 - runnerFrame;
    runnerChanged = true;
  }

  // 사람 위치 이동
  if (now - lastMoveTime >= MOVE_INTERVAL) {
    lastMoveTime = now;

    // 이전 위치 지우기
    lcd.setCursor(runnerColumn, 0);
    lcd.print(' ');

    // 0~15번 칸을 반복
    runnerColumn = (runnerColumn + 1) % 16;
    runnerChanged = true;
  }

  // 위치 또는 다리 모양이 바뀌었을 때만 표시
  if (runnerChanged) {
    lcd.setCursor(runnerColumn, 0);
    lcd.write(runnerFrame);
  }

  // 가변저항 막대 갱신
  if (now - lastBarTime >= BAR_INTERVAL) {
    lastBarTime = now;
    updateBar();
  }
}
