#pragma once

namespace todod::helpers {

template <class...Ts>
struct overloaded: ...Ts {
    using Ts::operator()...;
};

template <class...Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace todod::helpers