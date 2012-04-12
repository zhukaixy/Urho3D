//
// Urho3D Engine
// Copyright (c) 2008-2012 Lasse ��rni
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#include "CollisionShape.h"

/// Cylinder collision shape component.
class ConeShape : public CollisionShape
{
    OBJECT(ConeShape);
    
public:
    /// Construct.
    ConeShape(Context* context);
    /// Register object factory.
    static void RegisterObject(Context* context);
    
    /// %Set cylinder radius.
    void SetRadius(float radius);
    /// %Set cylinder height.
    void SetHeight(float height);
    
    /// Return cylinder radius.
    float GetRadius() const { return radius_; }
    /// Return cylinder height.
    float GetHeight() const { return height_; }
    
protected:
    /// Update the collision shape.
    virtual void UpdateCollisionShape();
    
private:
    /// Cylinder radius.
    float radius_;
    /// Cylinder height.
    float height_;
};