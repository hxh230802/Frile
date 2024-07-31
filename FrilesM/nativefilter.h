#include <QAbstractNativeEventFilter>

class NativeFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message,
        long *result);
};
