// Blood Donor Finder - DS Mini Project
// Data structure : Singly Linked List
// Algorithms     : Linear Search, Merge Sort (custom), Date difference (custom)
// Storage        : CSV file (donors.csv)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
using namespace std;

const int GAP_DAYS = 90;                 // minimum gap between two donations
const string FILE_NAME = "donors.csv";

// ---------------- Node ----------------
struct Donor {
    string name, group, city, phone;
    int d, m, y;                          // last donation date
    Donor *next;
};
Donor *head = nullptr;

// ---------------- Helpers ----------------
string upper(string s) {                  // normalise input: " o+ " -> "O+"
    string r;
    for (char c : s) if (c != ' ') r += toupper(c);
    return r;
}

// days since 1970-01-01 (custom, no library date functions)
long toDays(int d, int m, int y) {
    if (m <= 2) { y--; m += 12; }
    return 365L * y + y / 4 - y / 100 + y / 400 + (153 * (m - 3) + 2) / 5 + d;
}

long todayDays() {
    time_t t = time(nullptr);
    tm *now = localtime(&t);
    return toDays(now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
}

bool isEligible(Donor *p) {
    return todayDays() - toDays(p->d, p->m, p->y) >= GAP_DAYS;
}

void show(Donor *p) {
    cout << p->name << " | " << p->group << " | " << p->city << " | " << p->phone
         << " | last: " << p->d << "/" << p->m << "/" << p->y
         << " | " << (isEligible(p) ? "ELIGIBLE" : "NOT YET") << "\n";
}

// ---------------- Insert (at end) ----------------
void addDonor(Donor *n) {
    n->next = nullptr;
    if (!head) { head = n; return; }
    Donor *t = head;
    while (t->next) t = t->next;
    t->next = n;
}

void addDonorInteractive() {
    Donor *n = new Donor;
    cout << "Name: ";            getline(cin, n->name);
    cout << "Blood group (e.g. O+): "; getline(cin, n->group); n->group = upper(n->group);
    cout << "City: ";            getline(cin, n->city);      n->city = upper(n->city);
    cout << "Phone: ";           getline(cin, n->phone);
    cout << "Last donation (d m y): "; cin >> n->d >> n->m >> n->y; cin.ignore();
    addDonor(n);
    cout << "Donor added.\n";
}

// ---------------- Delete (by name) ----------------
void deleteDonor(const string &name) {
    Donor *cur = head, *prev = nullptr;
    while (cur && cur->name != name) { prev = cur; cur = cur->next; }
    if (!cur) { cout << "Not found.\n"; return; }
    if (!prev) head = cur->next;          // deleting the first node
    else prev->next = cur->next;
    delete cur;
    cout << "Deleted.\n";
}

// ---------------- Linear Search ----------------
// empty group/city means "don't filter on this field"
void search(const string &group, const string &city, bool onlyEligible) {
    int found = 0;
    for (Donor *p = head; p; p = p->next) {
        if (!group.empty() && p->group != group) continue;
        if (!city.empty() && p->city != city) continue;
        if (onlyEligible && !isEligible(p)) continue;
        show(p); found++;
    }
    if (!found) cout << "No matching donors.\n";
}

// ---------------- Merge Sort on linked list ----------------
// key 1 = last donation date (oldest first), key 2 = city (A-Z)
bool lessThan(Donor *a, Donor *b, int key) {
    if (key == 1) return toDays(a->d, a->m, a->y) <= toDays(b->d, b->m, b->y);
    return a->city <= b->city;
}

Donor *mergeLists(Donor *a, Donor *b, int key) {
    if (!a) return b;
    if (!b) return a;
    if (lessThan(a, b, key)) { a->next = mergeLists(a->next, b, key); return a; }
    b->next = mergeLists(a, b->next, key);
    return b;
}

void split(Donor *src, Donor **front, Donor **back) {   // slow/fast pointers
    Donor *slow = src, *fast = src->next;
    while (fast) {
        fast = fast->next;
        if (fast) { slow = slow->next; fast = fast->next; }
    }
    *front = src;
    *back = slow->next;
    slow->next = nullptr;
}

void mergeSort(Donor **headRef, int key) {
    Donor *h = *headRef, *a, *b;
    if (!h || !h->next) return;           // 0 or 1 node: already sorted
    split(h, &a, &b);
    mergeSort(&a, key);
    mergeSort(&b, key);
    *headRef = mergeLists(a, b, key);
}

// ---------------- File handling ----------------
void saveToFile() {
    ofstream f(FILE_NAME);
    for (Donor *p = head; p; p = p->next)
        f << p->name << "," << p->group << "," << p->city << "," << p->phone << ","
          << p->d << "," << p->m << "," << p->y << "\n";
}

void loadFromFile() {
    ifstream f(FILE_NAME);
    string line;
    while (getline(f, line)) {
        stringstream ss(line);
        string d, m, y;
        Donor *n = new Donor;
        getline(ss, n->name, ','); getline(ss, n->group, ',');
        getline(ss, n->city, ','); getline(ss, n->phone, ',');
        getline(ss, d, ',');       getline(ss, m, ',');       getline(ss, y, ',');
        n->d = stoi(d); n->m = stoi(m); n->y = stoi(y);
        addDonor(n);
    }
}

// ---------------- Menu ----------------
int main() {
    loadFromFile();
    int ch;
    do {
        cout << "\n--- BLOOD DONOR FINDER ---\n"
             << "1. Add donor\n2. Search donors\n3. Sort by last donation\n"
             << "4. Sort by city\n5. Show all\n6. Delete donor\n0. Save & Exit\nChoice: ";
        cin >> ch; cin.ignore();
        if (ch == 1) addDonorInteractive();
        else if (ch == 2) {
            string g, c; char e;
            cout << "Blood group (blank = any): "; getline(cin, g);
            cout << "City (blank = any): ";        getline(cin, c);
            cout << "Only eligible donors? (y/n): "; cin >> e; cin.ignore();
            search(upper(g), upper(c), e == 'y' || e == 'Y');
        }
        else if (ch == 3) { mergeSort(&head, 1); cout << "Sorted by last donation.\n"; }
        else if (ch == 4) { mergeSort(&head, 2); cout << "Sorted by city.\n"; }
        else if (ch == 5) { for (Donor *p = head; p; p = p->next) show(p); }
        else if (ch == 6) { string n; cout << "Name: "; getline(cin, n); deleteDonor(n); }
    } while (ch != 0);
    saveToFile();
    return 0;
}
