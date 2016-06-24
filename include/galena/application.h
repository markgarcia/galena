#pragma once


namespace galena {


class application {
public:
    bool is_open() const { return m_is_open; }
    void process_events();

private:
    bool m_is_open = true;
};


}
