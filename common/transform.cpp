#include "transform.hpp"
#include <glm/glm.hpp>

Transform::Transform(Transform* _parent)
    : parent(_parent)
{}

Transform::Transform(const glm::vec3& _s, const glm::vec3& _r, const glm::vec3& _t, Transform* _parent)
    : s(_s),
      r(_r),
      t(_t),
      parent(_parent)
{}

Transform::Transform(const Transform& _transform, Transform* _parent)
    : s(_transform.s),
      r(_transform.r),
      t(_transform.t),
      parent(_parent)
{}