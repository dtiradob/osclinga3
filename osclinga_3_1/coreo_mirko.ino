void coreoMirko() {
  
  if (tiempo < 420000) {
    preset1();
    estorbo(1, 40, 80, 60);
  }

  if ((tiempo > 420000) && (tiempo < 440000)) {
    estorbo(0, 20, 30, 30);
    preset2();
  }

  if ((tiempo > 440000) && (tiempo < 500000)) {
    delay((random(2000)) + 2000);
    preset3();
  }

  if ((tiempo > 500000) && (tiempo < 520000)) {
    preset20();
  }

  if ((tiempo > 520000) && (tiempo < 580000)) {
    delay((random(1000)) + 2000);
    preset4();
  }

  if ((tiempo > 580000) && (tiempo < 590000)) {
    preset5();
    estorbo(1, 40, 80, 60);
  }

  if ((tiempo > 590000) && (tiempo < 710000)) {
    estorbo(0, 20, 30, 30);
    preset6();
    estorbo2(1, 70, 60);
  }

  if ((tiempo > 710000) && (tiempo < 720000)) {
    estorbo2(0, 70, 60);
    preset50();
    estorbo(1, 40, 80, 60);
  }

  if ((tiempo > 720000) && (tiempo < 780000)) {
    apaga();
    preset7();
    estorback(1, 100, 2500, 100);
    estorbo(1, 40, 80, 60);
  }

  if ((tiempo > 780000) && (tiempo < 790000)) {
    estorback(0, 100, 2500, 100);
    preset500();
    estorbo(1, 40, 80, 60);
  }

  if ((tiempo > 790000) && (tiempo < 820000)) {
    estorbackfin(1, 100, 3000, 3000);
  }

  if (tiempo > 820000) {
    if (!wait) {
      estorbackfin(0, 100, 3000, 3000);
      preset0();
      wait = 1;
    }
  }

  if (wait) {
    agenda();
  }
}