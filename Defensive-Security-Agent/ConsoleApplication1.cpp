#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

using namespace std;
namespace fs = std::filesystem;

// Loglama sınıfı
class Logger {
private:
    string fileName;

public:
    Logger(const string& fileName) {
        this->fileName = fileName;
    }

    string getCurrentTime() {
        time_t now = time(0);
        tm localTime;
        localtime_s(&localTime, &now);

        char buffer[80];
        strftime(buffer, sizeof(buffer), "[%d.%m.%Y %H:%M:%S]", &localTime);

        return string(buffer);
    }

    void write(const string& message) {
        string finalMessage = getCurrentTime() + " " + message;

        cout << finalMessage << endl;

        ofstream file(fileName, ios::app);
        if (file.is_open()) {
            file << finalMessage << endl;
            file.close();
        }
    }
};

// Tehdit kuralı sınıfı
class ThreatRule {
private:
    vector<string> suspiciousProcessNames;

public:
    ThreatRule() {
        suspiciousProcessNames = {
            "keylogger.exe",
            "miner.exe",
            "trojan.exe",
            "backdoor.exe",
            "rat.exe"
        };
    }

    bool isSuspiciousProcess(const string& processName) {
        string lowerName = processName;
        transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        for (const string& rule : suspiciousProcessNames) {
            if (lowerName.find(rule) != string::npos) {
                return true;
            }
        }

        return false;
    }

    string getFileRiskLevel(const string& filePath) {
        string extension = fs::path(filePath).extension().string();
        transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

        if (extension == ".vbs")
            return "YUKSEK";

        if (extension == ".ps1")
            return "YUKSEK";

        if (extension == ".scr")
            return "YUKSEK";

        if (extension == ".bat")
            return "ORTA";

        if (extension == ".cmd")
            return "ORTA";

        return "TEMIZ";
    }
};

// Process tarama sınıfı
class ProcessScanner {
private:
    ThreatRule& rule;
    Logger& logger;

public:
    ProcessScanner(ThreatRule& rule, Logger& logger) : rule(rule), logger(logger) {}

    void scanProcesses() {
        int totalProcess = 0;
        int suspiciousProcess = 0;

        logger.write("=== Process taramasi baslatildi ===");

#ifdef _WIN32
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (snapshot == INVALID_HANDLE_VALUE) {
            logger.write("Process listesi alinamadi.");
            return;
        }

        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &processEntry)) {
            do {
                totalProcess++;

                wstring wideName = processEntry.szExeFile;
                string processName(wideName.begin(), wideName.end());

                if (rule.isSuspiciousProcess(processName)) {
                    suspiciousProcess++;
                    logger.write("[YUKSEK] Supheli process bulundu: " + processName);
                }
                else {
                    logger.write("[OK] Process: " + processName);
                }

            } while (Process32Next(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
#else
        logger.write("Bu ornekte process taramasi Windows icin aktif edilmistir.");
        logger.write("Linux/macOS icin /proc veya ps komutu ile genisletilebilir.");
#endif

        logger.write("=== Process taramasi tamamlandi ===");
        logger.write("Toplam process sayisi: " + to_string(totalProcess));
        logger.write("Supheli process sayisi: " + to_string(suspiciousProcess));
    }
};

// Dosya tarama sınıfı
class FileScanner {
private:
    ThreatRule& rule;
    Logger& logger;

public:
    FileScanner(ThreatRule& rule, Logger& logger) : rule(rule), logger(logger) {}

    void scanDirectory(const string& directoryPath) {
        int totalFile = 0;
        int riskyFile = 0;
        int mediumRisk = 0;
        int highRisk = 0;

        logger.write("=== Dosya taramasi baslatildi ===");

        if (!fs::exists(directoryPath)) {
            logger.write("Klasor bulunamadi: " + directoryPath);
            return;
        }

        try {
            for (const auto& entry : fs::recursive_directory_iterator(directoryPath)) {
                if (entry.is_regular_file()) {
                    totalFile++;

                    string filePath = entry.path().string();
                    string riskLevel = rule.getFileRiskLevel(filePath);

                    if (riskLevel != "TEMIZ") {
                        riskyFile++;

                        if (riskLevel == "ORTA")
                            mediumRisk++;

                        if (riskLevel == "YUKSEK")
                            highRisk++;

                        logger.write("[" + riskLevel + "] Riskli dosya bulundu: " + filePath);
                    }
                }
            }
        }
        catch (const exception& e) {
            logger.write(string("Dosya tarama hatasi: ") + e.what());
        }

        logger.write("=== Dosya taramasi tamamlandi ===");
        logger.write("Toplam taranan dosya: " + to_string(totalFile));
        logger.write("Toplam riskli dosya: " + to_string(riskyFile));
        logger.write("Orta riskli dosya: " + to_string(mediumRisk));
        logger.write("Yuksek riskli dosya: " + to_string(highRisk));
    }
};

// Ana güvenlik ajanı sınıfı
class SecurityAgent {
private:
    Logger logger;
    ThreatRule rule;
    ProcessScanner processScanner;
    FileScanner fileScanner;

public:
    SecurityAgent()
        : logger("C:\\Users\\karag\\Desktop\\security_report.txt"),
        rule(),
        processScanner(rule, logger),
        fileScanner(rule, logger) {
    }

    void showMenu() {
        cout << "\n====================================\n";
        cout << " DEFENSIVE SECURITY AGENT\n";
        cout << " OOP Tabanli C++ Guvenlik Uygulamasi\n";
        cout << "====================================\n";
        cout << "1 - Process Taramasi\n";
        cout << "2 - Dosya Taramasi\n";
        cout << "3 - Tam Guvenlik Taramasi\n";
        cout << "4 - Cikis\n";
        cout << "Seciminiz: ";
    }

    void start() {
        int choice;

        logger.write("====================================");
        logger.write("Defensive Cross-Platform Security Agent baslatildi.");
        logger.write("====================================");

        do {
            showMenu();
            cin >> choice;
            cin.ignore();

            if (choice == 1) {
                processScanner.scanProcesses();
            }
            else if (choice == 2) {
                string path;
                cout << "Taranacak klasor yolunu giriniz: ";
                getline(cin, path);

                fileScanner.scanDirectory(path);
            }
            else if (choice == 3) {
                string path;

                processScanner.scanProcesses();

                cout << "Taranacak klasor yolunu giriniz: ";
                getline(cin, path);

                fileScanner.scanDirectory(path);

                logger.write("Tam guvenlik taramasi tamamlandi.");
            }
            else if (choice == 4) {
                logger.write("Program kapatildi.");
                cout << "Program kapatiliyor..." << endl;
            }
            else {
                cout << "Gecersiz secim. Tekrar deneyiniz." << endl;
            }

        } while (choice != 4);

        logger.write("Rapor dosyasi: C:\\Users\\karag\\Desktop\\security_report.txt");
    }
};

int main() {
    SecurityAgent agent;
    agent.start();

    return 0;
}