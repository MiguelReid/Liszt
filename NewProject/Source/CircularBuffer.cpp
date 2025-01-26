template <typename T, size_t Size>
class CircularBuffer
{
public:
    CircularBuffer() : head(0), tail(0), isFull(false) {}

    bool push(const T& item)
    {
        buffer[head] = item;
        head = (head + 1) % Size;

        if (isFull)
        {
            tail = (tail + 1) % Size;
        }

        isFull = head == tail;
        return true;
    }

    bool pop(T& item)
    {
        if (empty())
        {
            return false;
        }

        item = buffer[tail];
        isFull = false;
        tail = (tail + 1) % Size;
        return true;
    }

    bool empty() const
    {
        return (!isFull && (head == tail));
    }

    bool full() const
    {
        return isFull;
    }

private:
    T buffer[Size];
    size_t head;
    size_t tail;
    bool isFull;
};
