import random


GENERATOR_PATTERNS = (
    "scan", "cyclic_working_set", "just_over_cache", "hot_cold", "zipf", "strong_zipf", "random",
    "burst", "switching_working_sets", "returning_working_sets", "alternating_regions",
    "drifting_popularity", "flash_crowd", "recent_history_reuse", "delayed_history_reuse",
    "periodic_hot_set", "three_frequency_tiers", "local_walk", "scan_with_reuse", "two_phase",
    "hot_noise"
)


class Generator:
    def __init__(self, seed, data_len, key_count):
        if data_len <= 0:
            raise ValueError("data_len must be > 0")
        if key_count <= 0:
            raise ValueError("key_count must be > 0")
        self.data_len = data_len
        self.key_count = key_count
        self.rng = random.Random(seed)

    def _random_key(self):
        return self.rng.randint(1, self.key_count)

    def _weighted_random(self, weights):
        total = sum(weights)
        cumulative = []
        acc = 0.0

        for weight in weights:
            acc += weight / total
            cumulative.append(acc)

        result = []
        for _ in range(self.data_len):
            x = self.rng.random()
            lo, hi = 0, self.key_count - 1

            while lo < hi:
                mid = (lo + hi) // 2
                if cumulative[mid] < x:
                    lo = mid + 1
                else:
                    hi = mid

            result.append(lo + 1)

        return result

    def scan(self, cache_size):
        return [(i % self.key_count) + 1 for i in range(self.data_len)]

    def cyclic_working_set(self, cache_size):
        ws_size = max(1, min(self.key_count, cache_size))
        working_set = list(range(1, ws_size + 1))
        return [working_set[i % ws_size] for i in range(self.data_len)]

    def just_over_cache(self, cache_size):
        ws_size = min(self.key_count, cache_size + 1)
        working_set = list(range(1, ws_size + 1))
        return [working_set[i % ws_size] for i in range(self.data_len)]

    def hot_cold(self, cache_size):
        hot_size = max(1, min(self.key_count, cache_size // 2, max(1, self.key_count // 4)))
        hot = list(range(1, hot_size + 1))
        cold = list(range(hot_size + 1, self.key_count + 1))
        result = []

        for _ in range(self.data_len):
            result.append(self.rng.choice(hot if not cold or self.rng.random() < 0.8 else cold))

        return result

    def zipf(self, cache_size):
        weights = [1.0 / (i ** 1.2) for i in range(1, self.key_count + 1)]
        return self._weighted_random(weights)

    def strong_zipf(self, cache_size):
        weights = [1.0 / (i ** 2.0) for i in range(1, self.key_count + 1)]
        return self._weighted_random(weights)

    def random(self, cache_size):
        return [self._random_key() for _ in range(self.data_len)]

    def burst(self, cache_size):
        result = []

        while len(result) < self.data_len:
            low = max(1, min(cache_size // 2, self.key_count))
            high = max(low, min(cache_size * 2, self.key_count))
            ws_size = self.rng.randint(low, high)
            start = self.rng.randint(1, self.key_count - ws_size + 1)
            working_set = list(range(start, start + ws_size))
            burst_len = self.rng.randint(max(1, cache_size), max(1, cache_size * 4))
            count = min(burst_len, self.data_len - len(result))
            result.extend(working_set[i % ws_size] for i in range(count))

        return result

    def switching_working_sets(self, cache_size):
        ws_size = max(1, min(self.key_count, cache_size))
        max_start = max(1, self.key_count - ws_size + 1)
        starts = [self.rng.randint(1, max_start) for _ in range(4)]
        sets = [list(range(start, start + ws_size)) for start in starts]
        result = []

        while len(result) < self.data_len:
            for working_set in sets:
                repetitions = self.rng.randint(max(1, cache_size), max(1, cache_size * 4))
                count = min(repetitions, self.data_len - len(result))
                result.extend(self.rng.choice(working_set) for _ in range(count))

        return result

    def returning_working_sets(self, cache_size):
        ws_size = max(1, min(cache_size, max(1, self.key_count // 3)))
        starts = [1, ws_size + 1, ws_size * 2 + 1]
        sets = []

        for start in starts:
            if start <= self.key_count:
                end = min(self.key_count, start + ws_size - 1)
                sets.append(list(range(start, end + 1)))

        if not sets:
            return self.scan(cache_size)

        result = []
        while len(result) < self.data_len:
            for working_set in sets:
                count = min(ws_size * 2, self.data_len - len(result))
                result.extend(self.rng.choice(working_set) for _ in range(count))

        return result

    def alternating_regions(self, cache_size):
        size = max(1, min(cache_size, max(1, self.key_count // 4)))
        left = list(range(1, size + 1))
        right_start = max(1, self.key_count // 2)
        right_end = min(self.key_count, right_start + size - 1)
        right = list(range(right_start, right_end + 1))

        if left == right:
            return self.returning_working_sets(cache_size)

        result = []
        while len(result) < self.data_len:
            for working_set in (left, right):
                count = min(size, self.data_len - len(result))
                result.extend(self.rng.choice(working_set) for _ in range(count))

        return result

    def drifting_popularity(self, cache_size):
        hot_size = max(1, min(cache_size, self.key_count))
        result = []
        center = 1
        step = max(1, self.data_len // 20)

        for i in range(self.data_len):
            if i > 0 and i % step == 0:
                center += max(1, hot_size // 2)
                if center + hot_size - 1 > self.key_count:
                    center = 1

            end = min(self.key_count, center + hot_size - 1)
            result.append(self.rng.randint(center, end))

        return result

    def flash_crowd(self, cache_size):
        hot_size = max(1, min(cache_size, self.key_count))
        max_start = max(1, self.key_count - hot_size + 1)
        flash_start = self.rng.randint(1, max_start)
        flash_set = list(range(flash_start, flash_start + hot_size))
        result = []

        for i in range(self.data_len):
            phase = i / self.data_len
            result.append(self.rng.choice(flash_set) if 0.4 <= phase <= 0.6 else self._random_key())

        return result

    def recent_history_reuse(self, cache_size):
        result = []
        history = []
        history_size = max(1, cache_size * 2)

        for _ in range(self.data_len):
            key = self.rng.choice(history) if history and self.rng.random() < 0.8 else \
                                                                                self._random_key()
            result.append(key)
            history.append(key)

            if len(history) > history_size:
                history.pop(0)

        return result

    def delayed_history_reuse(self, cache_size):
        result = []
        delay = max(1, min(self.data_len // 3, cache_size * 2))

        for i in range(self.data_len):
            if i >= delay and self.rng.random() < 0.6:
                result.append(result[i - delay])
            else:
                result.append(self._random_key())

        return result

    def periodic_hot_set(self, cache_size):
        hot_size = max(1, min(cache_size, self.key_count))
        hot = list(range(1, hot_size + 1))
        period = max(hot_size * 4, cache_size * 4)
        result = []

        for i in range(self.data_len):
            result.append(self.rng.choice(hot) if i % period < hot_size else self._random_key())

        return result

    def three_frequency_tiers(self, cache_size):
        if self.key_count == 1:
            return [1] * self.data_len

        hot_end = max(1, self.key_count // 10)
        warm_end = min(self.key_count, max(hot_end + 1, self.key_count // 2))
        result = []

        for _ in range(self.data_len):
            r = self.rng.random()

            if r < 0.60:
                result.append(self.rng.randint(1, hot_end))
            elif r < 0.90 and hot_end < warm_end:
                result.append(self.rng.randint(hot_end + 1, warm_end))
            elif warm_end < self.key_count:
                result.append(self.rng.randint(warm_end + 1, self.key_count))
            else:
                result.append(self.rng.randint(1, hot_end))

        return result

    def local_walk(self, cache_size):
        current = self.rng.randint(1, self.key_count)
        result = [current]

        for _ in range(self.data_len - 1):
            current += self.rng.choice([-3, -2, -1, 0, 1, 2, 3])
            current = max(1, min(self.key_count, current))
            result.append(current)

        return result

    def scan_with_reuse(self, cache_size):
        result = []
        reuse_limit = min(self.key_count, max(1, cache_size))

        for i in range(self.data_len):
            if self.rng.random() < 0.2:
                result.append(self.rng.randint(1, reuse_limit))
            else:
                result.append((i % self.key_count) + 1)

        return result

    def two_phase(self, cache_size):
        split = self.data_len // 2
        ws_size = max(1, min(cache_size, max(1, self.key_count // 3)))
        first_start = 1
        first_end = min(self.key_count, first_start + ws_size - 1)
        second_start = min(self.key_count, ws_size + self.key_count // 2)
        second_end = min(self.key_count, second_start + ws_size - 1)
        first = list(range(first_start, first_end + 1))
        second = list(range(second_start, second_end + 1))

        if not second:
            second = first

        result = [self.rng.choice(first) for _ in range(split)]
        result.extend(self.rng.choice(second) for _ in range(self.data_len - split))
        return result

    def hot_noise(self, cache_size):
        hot_size = max(1, min(cache_size, self.key_count))
        hot = list(range(1, hot_size + 1))
        result = []

        for _ in range(self.data_len):
            result.append(self.rng.choice(hot) if self.rng.random() < 0.7 else self._random_key())

        return result

    def generate(self, pattern, cache_size):
        if pattern not in GENERATOR_PATTERNS:
            raise ValueError(f"Unknown pattern: {pattern}")
        return getattr(self, pattern)(cache_size)

    def generate_all(self, cache_size):
        return {pattern: getattr(self, pattern)(cache_size) for pattern in GENERATOR_PATTERNS}
