#include <stdint.h>
#include "DB.h"
#include "Viruses.h"
#include "Scanner.h"
#include <memory>

class Timmer {
public:
	Timmer() = default;
	Timmer(const std::shared_ptr<DB>& db, const std::shared_ptr<Viruses>& viruses, const std::u16string path = u"", uint32_t hours = 0, uint32_t minutes = 0);
	Timmer& operator=(const Timmer& other);
	void setScanTime(uint32_t hours, uint32_t minutes);
	void setScanPath(const std::u16string& path);
	inline uint32_t getHours() { return hours; }
	inline uint32_t getMinutes() { return minutes; }
	inline std::u16string getPath() { return scanPath; }
	void start();
	inline void cancel() { shouldStop = true; }
	~Timmer() = default;

private:
	void timeMonitoring();

private:
	Scanner scanner;
	uint32_t hours = 0, minutes = 0;
	std::u16string scanPath;
	bool shouldStop = false;
};