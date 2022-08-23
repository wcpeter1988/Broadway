add_compile_options(
    -O3
    --memory-init-file 1
    --llvm-opts 3
    --llvm-lto 3
    -sNO_EXIT_RUNTIME=1
    -sNO_FILESYSTEM=1
    -sTOTAL_MEMORY=50MB
    -sINVOKE_RUN=0
    -sDOUBLE_MODE=0
    -sAGGRESSIVE_VARIABLE_ELIMINATION=1
    -sALIASING_FUNCTION_POINTERS=1
    -sDISABLE_EXCEPTION_CATCHING=1
    -sEXPORTED_FUNCTIONS=['HEAP8','HEAP16','HEAP32','_broadwayGetMajorVersion','_broadwayGetMinorVersion','_broadwayInit','_broadwayExit','_broadwayCreateStream','_broadwayPlayStream','_broadwayOnHeadersDecoded','_broadwayOnPictureDecoded']
    --js-library ../library.js
)

include_directories(../inc)

set(SOURCE_FILES
    h264bsd_transform.c
    h264bsd_util.c
    h264bsd_byte_stream.c
    h264bsd_seq_param_set.c
    h264bsd_pic_param_set.c
    h264bsd_slice_header.c
    h264bsd_slice_data.c
    h264bsd_macroblock_layer.c
    h264bsd_stream.c
    h264bsd_vlc.c
    h264bsd_cavlc.c
    h264bsd_nal_unit.c
    h264bsd_neighbour.c
    h264bsd_storage.c
    h264bsd_slice_group_map.c
    h264bsd_intra_prediction.c
    h264bsd_inter_prediction.c
    h264bsd_reconstruct.c
    h264bsd_dpb.c
    h264bsd_image.c
    h264bsd_deblocking.c
    h264bsd_conceal.c
    h264bsd_vui.c
    h264bsd_pic_order_cnt.c
    h264bsd_decoder.c
    H264SwDecApi.c
    extraFlags.c
    Decoder.c
    merge.cpp
)

add_executable(avc ${SOURCE_FILES})
set_target_properties(avc
    PROPERTIES SUFFIX ".js"
    LINK_FLAGS "
    --no-entry \
    -sNO_EXIT_RUNTIME=1 \
    -sNO_FILESYSTEM=1 \
    -sTOTAL_MEMORY=50MB \
    -sINVOKE_RUN=0 \
    -sDOUBLE_MODE=0 \
    -sAGGRESSIVE_VARIABLE_ELIMINATION=1 \
    -sALIASING_FUNCTION_POINTERS=1 \
    -sDISABLE_EXCEPTION_CATCHING=1 \
    -sEXPORTED_FUNCTIONS=['_broadwayGetMajorVersion','_broadwayGetMinorVersion','_broadwayInit','_broadwayExit','_broadwayCreateStream','_broadwayPlayStream','_broadwayOnHeadersDecoded','_broadwayOnPictureDecoded','_malloc','_free','_NV12Alpha2RGBA','_NV122RGBA','_RGBAlpha2RGBA'] \
    --js-library ../library.js")
# em_link_js_library(avc ${libraryJsFiles})

string(REGEX REPLACE "\/"
       "\\\\" PREJS
       "${CMAKE_CURRENT_SOURCE_DIR}/../../templates/DecoderPre.js")
string(REGEX REPLACE "\/"
       "\\\\" AVCJS
       "${CMAKE_CURRENT_BINARY_DIR}/avc.js")
string(REGEX REPLACE "\/"
       "\\\\" POSTJS
       "${CMAKE_CURRENT_SOURCE_DIR}/../../templates/DecoderPost.js")

add_custom_command(
    OUTPUT Decoder.js
    COMMAND type ${PREJS} ${AVCJS} ${POSTJS} > ${CMAKE_CURRENT_BINARY_DIR}/Decoder.js
    DEPENDS avc ${PREJS} ${POSTJS}
)
add_custom_target(
    Decoderjs
    DEPENDS Decoder.js
)
