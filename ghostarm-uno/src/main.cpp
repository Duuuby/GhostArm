#include <Arduino.h>
#include <math.h>

// --------------------------------------------------
// GhostArm EMG Prototype
// Alles in einer Datei: Input -> Filter -> Detektion -> Output
// --------------------------------------------------

// ---------- Pins ----------
constexpr uint8_t EMG_PIN    = A0;   // Analog-Eingang vom MyoWare
constexpr uint8_t OUTPUT_PIN = 8;    // LED / Testausgang / später Treiberstufe

// ---------- Sampling ----------
constexpr unsigned long SAMPLE_INTERVAL_US = 1000; // 1 kHz

// ---------- Filterparameter ----------
constexpr float DC_ALPHA  = 0.995f;  // langsame Basislinie
constexpr float ENV_ALPHA = 0.08f;   // Glättung der Hüllkurve

// ---------- Schwellwerte mit Hysterese ----------
constexpr int THRESHOLD_ON  = 80;    // Einschalten ab diesem Wert
constexpr int THRESHOLD_OFF = 60;    // Ausschalten erst unter diesem Wert

// ---------- Output-Verhalten ----------
constexpr unsigned long MIN_ON_TIME_MS = 80;   // minimale Einschaltdauer
constexpr unsigned long REFRACTORY_MS  = 120;  // kurze Sperrzeit nach Trigger

// ---------- Filter-Zustände ----------
float baseline = 512.0f;   // Startwert in der Mitte des ADC-Bereichs
float envelope = 0.0f;

// ---------- Detektions-Zustände ----------
bool outputActive = false;
unsigned long lastTriggerTime = 0;
unsigned long activeUntil = 0;

// ---------- Timing ----------
unsigned long lastSampleUs = 0;

// --------------------------------------------------
// Filterfunktion
// raw -> AC-Anteil -> gleichrichten -> Hüllkurve
// --------------------------------------------------
float processEMG(int rawValue) {
    // 1) Langsame Basislinie nachführen
    baseline = DC_ALPHA * baseline + (1.0f - DC_ALPHA) * rawValue;

    // 2) Wechselanteil berechnen
    float ac = rawValue - baseline;

    // 3) Gleichrichten
    float rectified = fabs(ac);

    // 4) Hüllkurve glätten
    envelope = ENV_ALPHA * rectified + (1.0f - ENV_ALPHA) * envelope;

    return envelope;
}

// --------------------------------------------------
// Kontraktion erkennen
// mit Hysterese + Mindest-Einschaltdauer + Sperrzeit
// --------------------------------------------------
bool detectContraction(float env, unsigned long nowMs) {
    // Solange Mindest-Einschaltdauer noch läuft: aktiv bleiben
    if (outputActive && nowMs < activeUntil) {
        return true;
    }

    // Prüfen, ob wir noch in der Sperrzeit nach dem letzten Trigger sind
    bool refractory = (nowMs - lastTriggerTime) < REFRACTORY_MS;

    if (!outputActive) {
        // Einschalten nur wenn aktuell aus, nicht in Sperrzeit und über ON-Schwelle
        if (!refractory && env >= THRESHOLD_ON) {
            outputActive = true;
            lastTriggerTime = nowMs;
            activeUntil = nowMs + MIN_ON_TIME_MS;
        }
    } else {
        // Ausschalten erst wenn unter OFF-Schwelle und Mindestzeit abgelaufen
        if (env <= THRESHOLD_OFF && nowMs >= activeUntil) {
            outputActive = false;
        }
    }

    return outputActive;
}

// --------------------------------------------------
// Setup
// --------------------------------------------------
void setup() {
    pinMode(EMG_PIN, INPUT);
    pinMode(OUTPUT_PIN, OUTPUT);
    digitalWrite(OUTPUT_PIN, LOW);

    Serial.begin(115200);
    delay(500);

    Serial.println("GhostArm EMG Prototype Start");
    Serial.println("raw,envelope,output");
}

// --------------------------------------------------
// Hauptschleife
// --------------------------------------------------
void loop() {
    unsigned long nowUs = micros();

    // feste Abtastrate
    if (nowUs - lastSampleUs >= SAMPLE_INTERVAL_US) {
        lastSampleUs = nowUs;

        // 1) Input lesen
        int raw = analogRead(EMG_PIN);

        // 2) Filtern
        float env = processEMG(raw);

        // 3) Kontraktion erkennen
        bool muscleActive = detectContraction(env, millis());

        // 4) Output setzen
        digitalWrite(OUTPUT_PIN, muscleActive ? HIGH : LOW);

        // 5) Debug-Ausgabe für Serial Plotter
        Serial.print(raw);
        Serial.print(",");
        Serial.print((int)env);
        Serial.print(",");
        Serial.println(muscleActive ? 1023 : 0);
    }
}