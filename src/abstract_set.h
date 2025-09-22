#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "id.h"

class AbstractSet {
private:
    struct Concept {
        virtual ~Concept() = default;
        virtual bool insert(id_t id) = 0;
        virtual bool contains(id_t id) const = 0;
        virtual size_t size() const = 0;
        virtual bool empty() const = 0;
        virtual void clear() = 0;
        virtual void for_each(std::function<void(id_t)> func) const = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    template<typename T>
    struct Model : Concept {
        T data;

        explicit Model(T t) : data(std::move(t)) {}

        bool insert(id_t id) override {
            return data.insert(id);
        }

        bool contains(id_t id) const override {
            return data.contains(id);
        }

        size_t size() const override {
            return data.size();
        }

        bool empty() const override {
            return data.empty();
        }

        void clear() override {
            data.clear();
        }

        void for_each(std::function<void(id_t)> func) const override {
            for (auto it = data.begin(); it != data.end(); ++it) {
                func(*it);
            }
        }

        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model<T>>(data);
        }
    };

    std::unique_ptr<Concept> impl;

public:
    template<typename T>
    explicit AbstractSet(T t) : impl(std::make_unique<Model<T>>(std::move(t))) {}

    AbstractSet(const AbstractSet& other) : impl(other.impl->clone()) {}
    AbstractSet(AbstractSet&& other) noexcept = default;

    AbstractSet& operator=(const AbstractSet& other) {
        if (this != &other) {
            impl = other.impl->clone();
        }
        return *this;
    }

    AbstractSet& operator=(AbstractSet&& other) noexcept = default;

    bool insert(id_t id) {
        return impl->insert(id);
    }

    bool contains(id_t id) const {
        return impl->contains(id);
    }

    size_t size() const {
        return impl->size();
    }

    bool empty() const {
        return impl->empty();
    }

    void clear() {
        impl->clear();
    }

    void for_each(std::function<void(id_t)> func) const {
        impl->for_each(func);
    }

    template<typename Func>
    void for_each(Func func) const {
        impl->for_each([&func](id_t id) { func(id); });
    }

    template<typename SetType>
    SetType copy_into() const {
        SetType result;
        for_each([&result](id_t id) {
            result.insert(id);
        });
        return result;
    }
};

// Intersection function for multiple sets
AbstractSet intersect(const std::vector<std::reference_wrapper<const AbstractSet>>& sets);

// Template function to copy a SetInterface into a specific set type
template<typename SetType>
SetType copy_into(const AbstractSet& source) {
    return source.copy_into<SetType>();
}
