#ifndef TICK_ARRAY_ARRAY2D_HPP_
#define TICK_ARRAY_ARRAY2D_HPP_

namespace statick {
namespace dense_2d {
template <class Archive, class A2D>
void save(Archive &ar, const A2D &a2d) {
  ar(CEREAL_NVP(false));
  ar(CEREAL_NVP(a2d.cols()));
  ar(CEREAL_NVP(a2d.rows()));
  ar(cereal::make_size_tag(a2d.size()));
  ar(cereal::binary_data(a2d.data(), a2d.size() * sizeof(typename A2D::value_type)));
}

template <typename T, class A2D>
void save_to(const A2D &a2d, const std::string &_file) {
  std::ofstream ss(_file, std::ios::out | std::ios::binary);
  cereal::PortableBinaryOutputArchive ar(ss);
  save(ar, a2d);
}
}  // namespace dense_2d


template <class T, class Archive>
bool load_array2d_with_raw_data(Archive &ar, std::vector<T> &data, std::vector<size_t> &info) {
  bool is_sparse = false;
  ar(is_sparse);
  if (is_sparse) throw std::runtime_error("Should not happen - array2d is sparse");
  size_t n_cols = 0, n_rows = 0, vectorSize = 0;
  ar(n_cols, n_rows);
  ar(cereal::make_size_tag(vectorSize));
  if ((n_cols * n_rows) != vectorSize)
    throw std::runtime_error("Should not happen - array2d size mismatch");
  if (data.size() < vectorSize) data.resize(vectorSize);
  ar(cereal::binary_data(data.data(), static_cast<std::size_t>(vectorSize) * sizeof(T)));
  info.resize(3);
  info[0] = n_cols;
  info[1] = n_rows;
  info[2] = vectorSize;
  return true;
}

template <typename T>
class Array2D {
 public:
  using value_type = T;
  Array2D() {}
  Array2D(size_t rows, size_t cols) : m_data(rows * cols), m_info(3) {
    m_info[0] = cols;
    m_info[1] = rows;
    m_info[2] = cols * rows;
  }
  Array2D(Array2D &&that) : m_data(that.m_data), m_info(that.m_info) {}

  const T *data() const { return m_data.data(); }
  RawArray<T> row(size_t i) const { return RawArray<T>(&m_data[i * m_info[0]], m_info[0]); }
  const T *row_raw(size_t i) const { return &m_data[i * m_info[0]]; }
  const T &operator[](int i) const { return m_data[i]; }

  const size_t &cols() const { return m_info[0]; }
  const size_t &rows() const { return m_info[1]; }
  const size_t &size() const { return m_info[2]; }
  const size_t *info() const { return m_info.data(); }

  static std::shared_ptr<Array2D<T>> FROM_CEREAL(const std::string &file) {
    auto array = std::make_shared<Array2D<T>>();
    {
      std::ifstream bin_data(file, std::ios::in | std::ios::binary);
      cereal::PortableBinaryInputArchive iarchive(bin_data);
      if (statick::load_array2d_with_raw_data(iarchive, array->m_data, array->m_info))
        return std::move(array);
    }
    return array;
  }

  static std::shared_ptr<Array2D<T>> RANDOM(size_t rows, size_t cols, T seed = -1) {
    std::random_device r;
    std::seed_seq seed_seq{r(), r(), r(), r(), r(), r(), r(), r()};
    std::mt19937_64 generator(seed_seq);
    std::uniform_real_distribution<T> uniform_dist;
    auto arr = std::make_shared<Array2D<T>>(rows, cols);
    for (size_t i = 0; i < arr->size(); i++) arr->m_data[i] = uniform_dist(generator);
    return arr;
  }

  template <class Archive>
  void load(Archive &ar) { load_array2d_with_raw_data(ar, m_data, m_info); }

  template <class Archive>
  void save(Archive &ar) const { dense_2d::save<T>(ar, *this); }

  std::vector<T> m_data;
  std::vector<size_t> m_info;

 private:
  Array2D(Array2D &that) = delete;
  Array2D(const Array2D &that) = delete;
  Array2D(const Array2D &&that) = delete;
  Array2D &operator=(Array2D &that) = delete;
  Array2D &operator=(Array2D &&that) = delete;
  Array2D &operator=(const Array2D &that) = delete;
  Array2D &operator=(const Array2D &&that) = delete;
};

template <typename T, typename I = size_t>
class RawArray2D {
 public:
  using value_type = T;
  RawArray2D(const T *_data, const I *_info) : v_data(_data), m_info(3) {
    m_info[0] = _info[0];
    m_info[1] = _info[1];
    m_info[2] = _info[0] * _info[1];
  }
  RawArray2D(RawArray2D &&that) : v_data(that.v_data), m_info(that.m_info) {}

  const T &operator[](int i) { return v_data[i]; }
  const T *data() const { return v_data; }
  const T *row_raw(size_t i) const { return &v_data[i * (m_info[0])]; }
  RawArray<T> row(size_t i) const { return RawArray<T>(&v_data[i * (m_info[0])], m_info[0]); }

  const I &cols() const { return m_info[0]; }
  const I &rows() const { return m_info[1]; }
  const I &size() const { return m_info[2]; }

  template <class Archive>
  void load(Archive &ar) { load_array2d_with_raw_data(ar, v_data, m_info); }

  template <class Archive>
  void save(Archive &ar) const { dense_2d::save<T>(ar, *this); }

 private:
  const T *v_data;
  std::vector<I> m_info;

  RawArray2D() = delete;
  RawArray2D(RawArray2D &that) = delete;
  RawArray2D(const RawArray2D &that) = delete;
  RawArray2D(const RawArray2D &&that) = delete;
  RawArray2D &operator=(RawArray2D &that) = delete;
  RawArray2D &operator=(RawArray2D &&that) = delete;
  RawArray2D &operator=(const RawArray2D &that) = delete;
  RawArray2D &operator=(const RawArray2D &&that) = delete;
};

template <class Archive, class T>
bool load_array2dlist_with_raw_data(Archive &ar, std::vector<T> &data, std::vector<size_t> &info) {
  bool is_sparse = false;
  size_t cols = 0, rows = 0, vectorSize = 0;
  ar(is_sparse);
  if (is_sparse) return false;
  ar(cols, rows);
  ar(cereal::make_size_tag(vectorSize));
  if ((cols * rows) != vectorSize) return false;
  if (data.size() < vectorSize) data.resize(vectorSize);
  ar(cereal::binary_data(data.data(), static_cast<std::size_t>(vectorSize) * sizeof(T)));
  info.resize(info.size() + 4);
  info[0] = cols;
  info[1] = rows;
  info[2] = vectorSize;
  info[3] = data.size() - vectorSize;
  return true;
}

template <typename T>
class Array2DList {
 private:
  static constexpr size_t INFO_SIZE = 4;
  static bool FROM_FILE(Array2DList &list, const std::string &&file) {
    std::ifstream bin_data(file, std::ios::in | std::ios::binary);
    cereal::PortableBinaryInputArchive iarchive(bin_data);
    return statick::load_array2dlist_with_raw_data(iarchive, list.m_data, list.m_info);
  }

 public:
  Array2DList() {}
  RawArray2D<T> operator[](size_t i) const {
    return RawArray2D<T>(m_data.data() + (m_info[(i * INFO_SIZE) + 3]), &m_info[i * INFO_SIZE]);
  }

  const size_t *info() const { return m_info.data(); }
  const T *data() const { return m_data.data(); }
  size_t size() const { return m_data.size(); }

  bool add_cereal(const std::string &file) { return FROM_FILE(*this, file); }

  static std::shared_ptr<Array2DList<T>> FROM_CEREALS(const std::vector<std::string> &&files) {
    auto array = std::make_shared<Array2DList<T>>();
    for (const auto &file : files)
      if (!FROM_FILE(*array.get(), file)) return nullptr;
    return std::move(array);
  }

 private:
  std::vector<T> m_data;
  std::vector<size_t> m_info;

  Array2DList(Array2DList &that) = delete;
  Array2DList(const Array2DList &that) = delete;
  Array2DList(Array2DList &&that) = delete;
  Array2DList(const Array2DList &&that) = delete;
  Array2DList &operator=(Array2DList &that) = delete;
  Array2DList &operator=(Array2DList &&that) = delete;
  Array2DList &operator=(const Array2DList &that) = delete;
  Array2DList &operator=(const Array2DList &&that) = delete;
};

template <typename T, typename ARRAY = Array2D<T>>
class SharedArray2DList {
 public:
  SharedArray2DList() {}
  SharedArray2DList(size_t size) : m_data(size) {}

  auto size() const { return m_data.size(); }
  auto empty() const { return m_data.empty(); }

  void push_back(std::shared_ptr<ARRAY> arr) { m_data.push_back(arr); }
  void add_at(std::shared_ptr<ARRAY> &arr, size_t i) { m_data[i] = arr; }

  auto &operator[](size_t index) { return m_data[index]; }
  const auto &operator[](size_t index) const  { return m_data[index]; }

 private:
  std::vector<std::shared_ptr<ARRAY>> m_data;

  SharedArray2DList(SharedArray2DList &that) = delete;
  SharedArray2DList(const SharedArray2DList &that) = delete;
  SharedArray2DList(SharedArray2DList &&that) = delete;
  SharedArray2DList(const SharedArray2DList &&that) = delete;
  SharedArray2DList &operator=(SharedArray2DList &that) = delete;
  SharedArray2DList &operator=(SharedArray2DList &&that) = delete;
  SharedArray2DList &operator=(const SharedArray2DList &that) = delete;
  SharedArray2DList &operator=(const SharedArray2DList &&that) = delete;
};

template <typename T>
using SharedRawArray2DList = SharedArray2DList<T, RawArray2D<T>>;


}  // namespace statick
#endif  //  TICK_ARRAY_ARRAY2D_HPP_
