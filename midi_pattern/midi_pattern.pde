import promidi.*;
MidiIO midiIO;

int x = 0;
int y = 0;
int a = 0;
int b = 0;
int lastA;
int lastB;
int changeSize = 15;
int selector = 6;
int colorCC;
int modulator = 0;

void setup() {
  colorMode(HSB, 127,  127, 127);
  size(800, 800);
  smooth();
  background(0);
  frameRate(30);
  strokeWeight(10);
  stroke(80, 127, 127);
  midiIO = MidiIO.getInstance(this);
  midiIO.printDevices();
  midiIO.plug(this, "noteOn", 9, 0);
  midiIO.plug(this, "noteOff", 9, 0);
  midiIO.plug(this, "controllerIn", 9, 0);
}

void draw() {
  fill(0, 0, 0);
}

void keyPressed() {
  if (key == CODED) {
    if (keyCode == UP) {
      fill(0);
      rect(0, 0, 800, 800);
      fill(255);
      a = 0;
      b = 0;
    }
    if (keyCode == RIGHT) {
      selector = selector +1;
    }
  }
  if (selector > 6) selector = 0;
}

void noteOn(Note note) {
  int vel = note.getVelocity();
  int pit = note.getPitch();
  println(pit);
  if (pit == 59 && vel == 90) {
    x = 0;
    y = -changeSize;
  }    
  if (pit == 60 && vel == 90) {
    x = changeSize;
    y = -changeSize;
  }
  if (pit == 60 && vel == 90) {
    x = changeSize;
    y = 0;
  }
  if (pit == 62 && vel == 90) {
    x = changeSize;
    y = changeSize;
  }
  if (pit == 63 && vel == 90) {
    x = 0;
    y = changeSize;
  }
  if (pit == 64 && vel == 90) {
    x = -changeSize;
    y = changeSize;
  } 
  if (pit == 65 && vel == 90) {
    x = -changeSize;
    y = 0;
  }
  if (pit == 66 && vel == 90) {
    x = -changeSize;
    y = -changeSize;
  } 

  a = a + x;
  b = b + y;
  
  stroke(colorCC, 127, 127);
  
  switch(selector) {
  case 0:
    point(a, b);
    break;
  case 1:
    line(lastA, lastB, a, b);
    break;
  case 2:
    line(800-a, 800-a, a, b);
    break;      
  case 3:
    line(800-a, 800-a, a, b);
    line(800-b, 800-b, a, b);
    break;
  case 4:
    line(800-a, 800-a, a, b);
    line(800-b, 800-b, a, b);
    line(800-a, a, 800-a, b);
    line(800-b, b, 800-b, b);
    break; 
  case 5: 
    line(0, 0, a, b);
    line(800, 0, a, b);
    line(0, 800, a, b);
    line(800, 800, a, b);
    break;
  case 6:
    rect(a, b, 50, 50);
    break;
  }

  lastA = a;
  lastB = b;

  if (a > 800) {
    a = modulator;
    lastA = modulator;
    modulator++;
  }
  if (b > 800) {
    b = modulator;
    lastB = modulator;
    modulator++;
  }
  if (a < 0) {
    a = 800;
    lastA = 800;
  }
  if (b < 0) {
    b = 800;
    lastB = 800;
  }
}

void noteOff(Note note) {
  int pit = note.getPitch();
}

void controllerIn(Controller controller) {
  int num = controller.getNumber();
  int val = controller.getValue();
  
  if (num == 20) {
    colorCC = val;
  }
    
}

