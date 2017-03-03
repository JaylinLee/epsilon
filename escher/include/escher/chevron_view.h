#ifndef ESCHER_CHEVRON_VIEW_H
#define ESCHER_CHEVRON_VIEW_H

#include <escher/view.h>

class ChevronView : public View {
public:
  ChevronView();
  void drawRect(KDContext * ctx, KDRect rect) const override;
  KDSize minimalSizeForOptimalDisplay() override;
  /* k_chevronHeight and k_chevronWidth are the dimensions of the chevron. */
  constexpr static KDCoordinate k_chevronHeight = 10;
  constexpr static KDCoordinate k_chevronWidth = 8;
private:
  constexpr static KDCoordinate k_chevronRightMargin = 10;
};

#endif
