from enum import StrEnum


class CapacitySharingPolicy(StrEnum):
    EQUAL = "equal"
    GEOMETRIC = "geometric"


def calc_cache_levels_capacities(cache_size, cache_levels, policy):
    match policy:
        case CapacitySharingPolicy.GEOMETRIC:
            result = []
            start = cache_size // (2 ** cache_levels - 1)
            for _ in range(cache_levels):
                result.append(start)
                start *= 2
            return result
        case CapacitySharingPolicy.EQUAL:
            return [cache_size // cache_levels] * cache_levels
