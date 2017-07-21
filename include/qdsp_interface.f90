module qdsp_interface
!F90 interface module
!Create pointers to title,x,y,color with c_loc() in iso_c_binding
!These arrays/strings must be declared with 'target' attribute and must not be allocatable

  implicit none
  
  interface

    type(c_ptr) function qdspInit(title) bind(C,name='qdspInit')
      use iso_c_binding, only: c_ptr,c_char
      type(c_ptr),value,intent(in) :: title
    end function

    integer(kind=c_int) function qdspUpdateIfReady(plot,x,y,color,part_num) bind(C,name='qdspUpdateIfReady')
      use iso_c_binding, only: c_ptr,c_int
      type(c_ptr),value :: plot,x,y,color
      integer(kind=c_int),value :: part_num
    end function

    subroutine qdspSetBounds(plot,xmin,xmax,ymin,ymax) bind(C,name='qdspSetBounds')
      use iso_c_binding, only: c_ptr,c_double
      type(c_ptr),value :: plot
      real(kind=c_double),value :: xmin,xmax,ymin,ymax
    end subroutine

    subroutine qdspSetGridX(plot,point,interval,rgb) bind(C,name='qdspSetGridX')
      use iso_c_binding, only: c_ptr,c_double,c_int
      type(c_ptr),value :: plot
      real(kind=c_double),value :: point,interval
      integer(kind=c_int),value :: rgb
    end subroutine

    subroutine qdspSetGridY(plot,point,interval,rgb) bind(C,name='qdspSetGridY')
      use iso_c_binding, only: c_ptr,c_double,c_int
      type(c_ptr),value :: plot
      real(kind=c_double),value :: point,interval
      integer(kind=c_int),value :: rgb
    end subroutine

    subroutine qdspSetPointColor(plot,rgb) bind(C,name='qdspSetPointColor')
      use iso_c_binding, only: c_ptr,c_int
      type(c_ptr),value :: plot
      integer(kind=c_int),value :: rgb
    end subroutine

    subroutine qdspSetBGColor(plot,rgb) bind(C,name='qdspSetBGColor')
      use iso_c_binding, only: c_ptr,c_int
      type(c_ptr),value :: plot
      integer(kind=c_int),value :: rgb
    end subroutine

  end interface 

end module
