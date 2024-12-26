#ifndef GLOBAL_H

#define DISABLE_COPY(Class) Class(Class const &) = delete; \
                            Class& operator=(Class const &) = delete;

#endif // !GLOBAL_H